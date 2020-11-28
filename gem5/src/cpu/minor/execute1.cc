/*
 * Copyright (c) 2013-2014 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cpu/minor/execute1.hh"

#include "cpu/minor/pipeline.hh"

namespace Minor
{

Execute1::Execute1(const std::string &name,
    MinorCPU &cpu_,
    MinorCPUParams &params,
    Latch<ForwardInstData>::Output inp_,
    Latch<ForwardInstData>::Input out_,
    std::vector<InputBuffer<ForwardInstData>> &next_stage_input_buffer) :
    Named(name),
    cpu(cpu_),
    inp(inp_),
    out(out_),
    nextStageReserve(next_stage_input_buffer),
    outputWidth(params.executeInputWidth),       
    processMoreThanOneInput(params.execute1CycleInput),
    execute1Info(params.numThreads),
    threadPriority(0)
{
    if (outputWidth < 1)
        fatal("%s: executeInputWidth must be >= 1 (%d)\n", name, outputWidth);

    if (params.execute1InputBufferSize < 1) {
        fatal("%s: execute1InputBufferSize must be >= 1 (%d)\n", name,
        params.execute1InputBufferSize);
    }

    /* Per-thread input buffers */
    for (ThreadID tid = 0; tid < params.numThreads; tid++) {
        inputBuffer.push_back(
            InputBuffer<ForwardInstData>(
                name + ".inputBuffer" + std::to_string(tid), "insts",
                params.execute1InputBufferSize));
    }
}

const ForwardInstData *
Execute1::getInput(ThreadID tid)
{
    /* Get insts from the inputBuffer to work with */
    if (!inputBuffer[tid].empty()) {
        const ForwardInstData &head = inputBuffer[tid].front();

        return (head.isBubble() ? NULL : &(inputBuffer[tid].front()));
    } else {
        return NULL;
    }
}

void
Execute1::popInput(ThreadID tid)
{
    if (!inputBuffer[tid].empty())
        inputBuffer[tid].pop();

    execute1Info[tid].inputIndex = 0;
//    execute1Info[tid].inMacroop = false;
}

#if TRACING_ON
/** Add the tracing data to an instruction.  This originates in
 *  decode because this is the first place that execSeqNums are known
 *  (these are used as the 'FetchSeq' in tracing data) */
static void
dynInstAddTracing(MinorDynInstPtr inst, StaticInstPtr static_inst,
    MinorCPU &cpu)
{
    inst->traceData = cpu.getTracer()->getInstRecord(curTick(),
        cpu.getContext(inst->id.threadId),
        inst->staticInst, inst->pc, static_inst);

    /* Use the execSeqNum as the fetch sequence number as this most closely
     *  matches the other processor models' idea of fetch sequence */
    if (inst->traceData)
        inst->traceData->setFetchSeq(inst->id.execSeqNum);
}
#endif

void
Execute1::evaluate()
{
    /* Push input onto appropriate input buffer */
    if (!inp.outputWire->isBubble())
        inputBuffer[inp.outputWire->threadId].setTail(*inp.outputWire);

    ForwardInstData &insts_out = *out.inputWire;

    assert(insts_out.isBubble());

    for (ThreadID tid = 0; tid < cpu.numThreads; tid++)
        execute1Info[tid].blocked = !nextStageReserve[tid].canReserve();

    ThreadID tid = getScheduledThread();

    if (tid != InvalidThreadID) {
        Execute1ThreadInfo &execute1_info = execute1Info[tid];
        const ForwardInstData *insts_in = getInput(tid);

        unsigned int output_index = 0;

        /* Pack instructions into the output while we can.  This may involve
         * using more than one input line */
        while (insts_in &&
           execute1_info.inputIndex < insts_in->width() && /* Still more input */
           output_index < outputWidth /* Still more output to fill */)
        {

            MinorDynInstPtr inst = insts_in->insts[execute1_info.inputIndex];
            if (inst->isBubble()) {
                /* Skip */
                execute1_info.inputIndex++;
            } else {
                StaticInstPtr static_inst = inst->staticInst;
                /* Static inst of a macro-op above the output_inst */
                StaticInstPtr parent_static_inst = NULL;
               
                MinorDynInstPtr output_inst = inst;
                /* Doesn't need decomposing, pass on instruction */
    //            DPRINTF(Execute1, "Passing on inst: %s inputIndex:"
    //                " %d output_index: %d\n",
    //                *output_inst, execute1_info.inputIndex, output_index);

                parent_static_inst = static_inst;

                /* Step input */
                execute1_info.inputIndex++;
                //execute1_info.inMacroop = false;
                
                /* Set execSeqNum of output_inst */
                output_inst->id.execSeqNum = execute1_info.execSeqNum;
                /* Add tracing */
#if TRACING_ON
                dynInstAddTracing(output_inst, parent_static_inst, cpu);
#endif

                /* Step to next sequence number */
                execute1_info.execSeqNum++;

                /* Correctly size the output before writing */
                if (output_index == 0) insts_out.resize(outputWidth);
                /* Push into output */
                insts_out.insts[output_index] = output_inst;
                output_index++;
            }
            /* Have we finished with the input? */
            if (execute1_info.inputIndex == insts_in->width()) {
                /* If we have just been producing micro-ops, we *must* have
                 * got to the end of that for inputIndex to be pushed past
                 * insts_in->width() */
                popInput(tid);
                insts_in = NULL;

                if (processMoreThanOneInput) {
//                    DPRINTF(Decode, "Wrapping\n");
                    insts_in = getInput(tid);
                }
            }
        }
    }

    /* If we generated output, reserve space for the result in the next stage
     *  and mark the stage as being active this cycle */
    if (!insts_out.isBubble()) {
        /* Note activity of following buffer */
        cpu.activityRecorder->activity();
        insts_out.threadId = tid;
        nextStageReserve[tid].reserve();
    }

    /* If we still have input to process and somewhere to put it,
     *  mark stage as active */
    for (ThreadID i = 0; i < cpu.numThreads; i++)
    {
        if (getInput(i) && nextStageReserve[i].canReserve()) {
            cpu.activityRecorder->activateStage(Pipeline::Execute1StageId);
            break;
        }
    }

    /* Make sure the input (if any left) is pushed */
    if (!inp.outputWire->isBubble())
        inputBuffer[inp.outputWire->threadId].pushTail();
}

inline ThreadID
Execute1::getScheduledThread()
{
    /* Select thread via policy. */
    std::vector<ThreadID> priority_list;

    switch (cpu.threadPolicy) {
      case Enums::SingleThreaded:
        priority_list.push_back(0);
        break;
      case Enums::RoundRobin:
        priority_list = cpu.roundRobinPriority(threadPriority);
        break;
      case Enums::Random:
        priority_list = cpu.randomPriority();
        break;
      default:
        panic("Unknown fetch policy");
    }

    for (auto tid : priority_list) {
        if (getInput(tid) && !execute1Info[tid].blocked) {
            threadPriority = tid;
            return tid;
        }
    }

   return InvalidThreadID;
}

bool
Execute1::isDrained()
{
    for (const auto &buffer : inputBuffer) {
        if (!buffer.empty())
            return false;
    }

    return (*inp.outputWire).isBubble();
}

void
Execute1::minorTrace() const
{
    std::ostringstream data;

    if (execute1Info[0].blocked)
        data << 'B';
    else
        (*out.inputWire).reportData(data);

    MINORTRACE("insts=%s\n", data.str());
    inputBuffer[0].minorTrace();
}

}
