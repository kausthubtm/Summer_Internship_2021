Abstract : This document just breifly summerizes the concepts I covered during weeks 1 & 2. It mainly consists the basics of the architetures of CPU and GPU.
<br />
<br />

# CPU Architecture
This section basically covers the topics that are taught in the Computer Organisation and Architecture course.  
<br />
<br />

## Von Neumann Architecture
It is the most basic arrangement where the computer system is diveded into three which are - the processor, the memory, and the I/O devices.

- **Processor :** Processor two important units called the Control Unit(CU) which handles control signals for I/O, fetching and controlling of data, and the Arithemtic and Logical Unit (ALU) which handles handles arithmetic and logical operations. Along with these the processor has registers to store intructions, data and critical for the execution of an instruction.

- **Registers :** They are made up of flip-flops for storing data. The registers used for storing the operands for operations and other data used by the instruction are categorised as General Purpose Registers. The Accumulator is the most commonly used register which comes under Genral Purpose Registers.The registers which help in instruction executionare called as Special Purpose Registers. There are a bunch of these registers and all of them are important :D . First we have the Program Counter (PC) which points to the nextto the next instruction to be fetched, then the Instruction Register (IR) which holds the instruction to be executed and for decoding. MAR is Memory Address Register which is used for storing the address from which the data has to fetched. MDR is Memory Data Register is used to stored the data that has been fetched from the memory. There is another set of registers Control Registers which act like flags for overflow, carry, negative, zero etc..

- **Memory :** The hierarchy is cache (L1, L2, L3) then Main Memory (DRAM) and the last one Auxilary Memory / Secondary Memory (Magnetic disks and tapes). The access time and the cost follow the order cache > Main Memory > Auxilary Memory. Main memory access is quite slow and this causes a bottleneck and stops us from utilzing the hardware to the fullest. This problem is discussed elborately in the the coming weeks.

- **I/O devices :** The I/O devices are input-output devices and concerns mainly with iterrupts and how to handle them. 

<br />
<br />

## RISC and CISC Architecture
  
- **RISC Architecture :** Reduced Instruction Set Computing are simple instructions which are single cycle instructions. Hence, this type of architecture is software intensive since the programmer would have to code each instruction step explicitly. RISC also provides less number of addressing modes. The RISC instructions for A = B * C would be :

```
LOAD R2 addr(B);     
LOAD R3 addr(C);     
MUL R1 R2 R3;     
STORE addr(A) R1;
```

- **CISC Architecture :** Complex Instruction Set Computing are complex instructions which are multicycle instructions. Hence, this type of architecture is hardware intensive since the hardware has to execute and understand many instructions. CISC supports compound addressing modes. The CISC instruction for A = B * C would be :
```
MUL addr(A) addr(B) addr(C);
```
When we compare the RISC and CISC instruction for A = B * C , we could say RISC is a broken down simpler version od CISC :D since one complex CISC instruction is broken down into four simple RISC instructions. Also its worthy to note that all the operatons in RISC takes pace through registers and no data is directly accessed from the memory.

<br />
<br />

## CPU Datapath
CPU datapath specifies the path through which data follows during instruction execution. The elementary datapath is fetch->decode->execute i.e., 'fetches' instructions, 'decodes', and 'executes' it. If we consider the serial implementation of instructions where instructions are executed one followed by next then it is bound by the slowest instruction and instruction with max delay in order to accomodate all the instructions. This method is highly inefficient, and the solution for this is pipelining and multicycled instructions.

<br />
<br />

## Pipelining
Pipelining is a process of arranging the hardware elements of CPU such that the overall performance in increased. Simultaneous execution of more than one instruction takes place in a pipelined processor. This concept was explained beutifully by my professor using the example of a washing machine.<br/> <br/>
Consider three stages : Washer-30min (W), Dyer-40min (D), Folder-20min (F).

```
W D F | | | | | |
| | | W D F | | |
| | | | | | W D F
```

So washing three sets would take 3x(30+40+20) = 270min. This is not very effective use of machine because when we are doing one process the other two machines are ideal and its a waste of resource. What pipelining does is that it does washer of second set of clothes when first set of clothers is being dried. Hence, both the dryer and the washer is being used. 

```
W D F | | 
| W D F |
| | W D F 
```
So the time taken if we pipeling the process then time taken would be 170 min. The above is an example of a 3-stage pipelining. The 5-stage pipelining in CPU is:

- Fetch (IF) : IR <= Memory[PC] ;  PC = PC + 4 ; 
- Decode (ID) : Understand instruction semantics.
- Execute (EX) : Arithmetic/logical operations, jump/branch conditions. (based on instruction)
- Memory (MEM) : read/write from/to memory. (for load/store instructions)
- Write Back (WB) : Update register file.

Ideally if we use pipelining we could execute one instruction per clock cycle but its not practical due to pipeline hazards. These hazards are discussed in the next section.

<br />
<br />

## Pipelining Hazards
A condition that causes the pipeling to stall is called a hazard. there are threee types
- **Data hazard :** Any condition in which either the source or the destination operands of an instruction are not available at the time expected in the pipelining. So some operations has to delayed, and the pipeline stalls. Eg : ADD A, 4 and MUL B, A, 3. Here the computation of result of first instruction has to completed before the second indtruction is executed hence there is a stall and the second instruction has to wait till first one is completed. One solution is an hardware approach called Operand Forwarding where instead of fron the register file, the second instruction can get the data directly from the output of ALU after the previous instruction is completed. Software solution would be to introduce NOP instruction through the compiler. 
- **Contol hazard :** Its also called instruction hazard. A delay in the availability of an instruction causes the pipeline to stall. (cache miss)
- **Structural hazard :** The situation when two instructions require the use of a given hardware resource at the same time. Eg: Load X(R1), R2. 

<br />
<br />

## Cache
