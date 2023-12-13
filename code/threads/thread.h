// thread.h 
//	Data structures for managing threads.  A thread represents
//	sequential execution of code within a program.
//	So the state of a thread includes the program counter,
//	the processor registers, and the execution stack.
//	
// 	Note that because we allocate a fixed size stack for each
//	thread, it is possible to overflow the stack -- for instance,
//	by recursing to too deep a level.  The most common reason
//	for this occuring is allocating large data structures
//	on the stack.  For instance, this will cause problems:
//
//		void foo() { int buf[1000]; ...}
//
//	Instead, you should allocate all data structures dynamically:
//
//		void foo() { int *buf = new int[1000]; ...}
//
//
// 	Bad things happen if you overflow the stack, and in the worst 
//	case, the problem may not be caught explicitly.  Instead,
//	the only symptom may be bizarre segmentation faults.  (Of course,
//	other problems can cause seg faults, so that isn't a sure sign
//	that your thread stacks are too small.)
//	
//	One thing to try if you find yourself with seg faults is to
//	increase the size of thread stack -- ThreadStackSize.
//
//  	In this interface, forking a thread takes two steps.
//	We must first allocate a data structure for it: "t = new Thread".
//	Only then can we do the fork: "t->fork(f, arg)".
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef THREAD_H
#define THREAD_H

#include "copyright.h"
#include "utility.h"
#include "sysdep.h"

#include "machine.h"
#include "addrspace.h"

// CPU register state to be saved on context switch.  
// The x86 needs to save only a few registers, 
// SPARC and MIPS needs to save 10 registers, 
// the Snake needs 18,
// and the RS6000 needs to save 75 (!)
// For simplicity, I just take the maximum over all architectures.

#define MachineStateSize 75 


// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!
const int StackSize = (8 * 1024);	// in words


// Thread state
enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED };


// The following class defines a "thread control block" -- which
// represents a single thread of execution.
//
//  Every thread has:
//     an execution stack for activation records ("stackTop" and "stack")
//     space to save CPU registers while not running ("machineState")
//     a "status" (running/ready/blocked)
//    
//  Some threads also belong to a user address space; threads
//  that only run in the kernel have a NULL address space.


#define MAX_SIZE  128  //最大线程数
//pk数组主要记录线程号被使用的状态
//下标位线程号，数组对应值位线程号状态
//对应值为0时，表示该线程号没有线程占用
//对应值为1时，表示该线程号被占用
static int pk[MAX_SIZE]={0};
static int threadMAX = 0;//记录线程数

class Thread {
  private:
    // NOTE: DO NOT CHANGE the order of these first two members.
    // THEY MUST be in this position for SWITCH to work.
    int *stackTop;			 // the current stack pointer
    void *machineState[MachineStateSize];  // all registers except for stackTop

	int tid;
	int priority;//线程优先级,值越大优先级越高
  public:
    Thread(char* debugName);//原构造函数		// initialize a Thread 
    ~Thread(); //析构函数				// deallocate a Thread
					// NOTE -- thread being deleted
					// must not be running when delete 
					// is called

    // basic thread operations
	Thread(char* debugName,int priority);//新构造函数
	int getTid(){return this->tid;};//获得线程id
	int getPriority(){return priority;};//获得线程优先级

    void Fork(VoidFunctionPtr func, void *arg); //将线程加入就绪队列
    				// Make thread run (*func)(arg)
    void Yield();  //打断当前线程，运行就绪队列里的线程
					// Relinquish the CPU if any 
				// other thread is runnable
    void Sleep(bool finishing);//将当前线程阻塞
								 // Put the thread to sleep and 
				// relinquish the processor
    void Begin();	// Startup code for the thread	
    void Finish();  //线程运行结束	
						// The thread is done executing
    
    void CheckOverflow();   	// Check if thread stack has overflowed
    void setStatus(ThreadStatus st) { status = st; }//设置线程状态
    char* getName() { return (name); }//获取线程名字
    void Print() { cout << name; }//打印线程名字
    void SelfTest();	//测试方法	// test whether thread impl is working

  private:
    // some of the private data for this class is listed above
    
    int *stack; 	 	// Bottom of the stack 
				// NULL if this is the main thread
				// (If NULL, don't deallocate stack)
    ThreadStatus status;	// ready, running or blocked
    char* name;

    void StackAllocate(VoidFunctionPtr func, void *arg);
    				// Allocate a stack for thread.
				// Used internally by Fork()

// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.

    int userRegisters[NumTotalRegs];	// user-level CPU register state

  public:
    void SaveUserState();		// save user-level register state
    void RestoreUserState();		// restore user-level register state

    AddrSpace *space;			// User code this thread is running.
};

// external function, dummy routine whose sole job is to call Thread::Print
extern void ThreadPrint(Thread *thread);	 

// Magical machine-dependent routines, defined in switch.s

extern "C" {
// First frame on thread execution stack; 
//   	call ThreadBegin
//	call "func"
//	(when func returns, if ever) call ThreadFinish()
void ThreadRoot();

// Stop running oldThread and start running newThread
void SWITCH(Thread *oldThread, Thread *newThread);
}

#endif // THREAD_H

