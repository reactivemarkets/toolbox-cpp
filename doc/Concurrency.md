# Concurrency

## Concepts

Concurrent execution of code is concerned with:

- *Mutual Exclusion*: managing contention on a shared resource;
- *Memory Order*: the order in which memory operations are observed.

These concerns are typically addressed using locks and fences.

## Mutual Exclusion

Locks provide mutually exclusive access to a shared resource. In other words, access to a shared
resource is serialised, so that it cannot be accessed concurrently by multiple threads. According to
[Amdahl's law](https://en.wikipedia.org/wiki/Amdahl%27s_law), performance is limited by the amount
of sequential code on the critical path, so mutual exclusion must be minimised.

### Atomics

Atomic operations are indivisibility units of work that either happen entirely or not at all. When a
shared resource is modified using a single, atomic operation, such as a Compare And Swap (CAS)
instruction, mutual exclusion is kept minimal, which increases the potential performance gains.

## Memory Order

Memory order is concerned with the order in which memory operations occur both from the reader's and
writer's perspective. In the absence of memory ordering constraints, memory operations that occur in
program order can be reordered by both the compiler and processor. The following reorderings are
theoretically possible:

*LoadLoad*:
loads reordered with other loads.

*StoreStore*:
stores reordered with other stores.

*LoadStore*:
loads reordered with subsequent stores.

*StoreLoad*:
stores reordered with subsequent loads.

Memory order constraints prevent one or more of these reordering from happening across a memory
barrier or fence. These constraints are defined by the C11 and C++11 standards using more abstract
concepts: Relaxed, Consume, Acquire, Release, Acqrel and Seqcst.

N.B. Few compilers track the dependency chains required to implement Consume; most compilers simply
promote Consume to Acquire. Consequently, no further consideration is given to Consume in this
document.

Memory ordering semantics, as defined by the C11 and C++11 standards, are summarised below along
with their implied fences:

| memory order | fence                                   |
| ------------ | --------------------------------------- |
| Relaxed      | no-op                                   |
| Acquire      | LoadLoad+LoadStore                      |
| Release      | StoreStore+LoadStore                    |
| Acqrel       | LoadLoad+StoreStore+LoadStore           |
| Seqcst       | LoadLoad+StoreStore+LoadStore+StoreLoad |

These concepts are covered in more detail in the following sections.

### Atomics

Standalone fences impose memory order constraints between *all* memory operations before and after
the fence. A StoreLoad fence, for example, prevents any store before the fence from being reordered
with any load after the fence.

Atomic memory operations have the added benefit of associating fences with specific memory
operations, which loosens the restrictions and provides greater opportunities for optimisation.

In C11 and C++11, not all combinations of atomic memory operation and memory order constraint are
valid. Acquire is not valid with store operations, for example, and Release is not valid with load
operations.

### Relaxed

Relaxed provides no guarantees with respect to memory order, so it should only be used where there
are no data dependencies. Sequence numbers and reference counts are most commonly associated with
Relaxed semantics.

### Acquire

Acquire prevents LoadLoad and LoadStore reorderings, so that loads before the fence cannot be
reordered with loads and stores after the fence. Acquire is typically associated with an atomic load
before the fence:

``` asm
mov r1 [y]          ; Load.
#LoadLoad+LoadStore ; Acquire.
```

### Release

Release prevents StoreStore and LoadStore reorderings, so that stores after the fence cannot be
reordered with loads and stores before the fence. Release is typically associated with an atomic
store after the fence:

``` asm
#StoreStore+LoadStore ; Release.
mov [x] 1             ; Store.
```

### Seqcst

Seqcst prevents all possible reorderings including StoreLoad, which establishes a consistent order
of memory operations across all threads. Consider the following example of a ReleaseStore sequence
followed by a LoadAcquire:

``` asm
#StoreStore+LoadStore ; Release.
mov [x] 1             ; Store.
mov r1 [y]            ; Load.
#LoadLoad+LoadStore   ; Acquire.
```

Notice how sequential consistency can be broken by reordering the store and load operations. The
insertion of a StoreLoad fence prevents this:

``` asm
#StoreStore+LoadStore ; Release.
mov [x] 1             ; Store.
#StoreLoad
mov r1 [y]            ; Load.
#LoadLoad+LoadStore   ; Acquire.
```

The equivalent code using C++ atomics follows:

``` c++
x.store(1, memory_order_seqcst); // +StoreLoad.
auto r1 = y.load(memory_order_acquire);
```

Dekker's Algorithm, described later in this document, depends on stores happening before loads, and
so it is often cited as an example use-case for Seqcst.

### Acqrel

Unlike Acquire, Release and Seqcst, which are often used with standalone fences, Acqrel is almost
exclusively used with atomic Read Modify Write (RMW) operations, such add FetchAdd.

## Intel

One or more of these reorderings may not be possible in practice on memory architectures with
stronger memory models. Intel's "64 Architecture Memory Ordering" white paper, for example,
specifies the first four memory ordering principles as follows:

1.  loads are not reordered with other loads;
2.  stores are not reordered with other stores;
3.  stores are not reordered with older loads;
4.  loads may be reordered with older stores to different locations but not with older stores to the
    same location.

The first three disallow LoadLoad, StoreStore and LoadStore reordering, while the fourth allows
StoreLoad reordering. It is important to recognise, however, that without fences, compilers are free
to apply any of the four reorderings.

The following table shows instruction sequences typically used on Intel for various memory-model
combinations:

|          | Relaxed  | Acquire  | Release  | Acqrel   | Seqcst      |
| -------- | -------- | -------- | -------- | -------- | ----------- |
| Load     | mov      | mov      | N/A      | N/A      | mov         |
| Store    | mov      | N/A      | mov      | N/A      | mov; mfence |
| FetchAdd | lock add | lock add | lock add | lock add | lock add    |

Note that:

  - the `mfence` instruction is required to prevent StoreLoad reordering;
  - the typical cost of an `mfence` instruction is 100 cycles;
  - the `lock` prefix is effectively a full barrier.

Some Memory Architectures, such as IA64, have load and store forms that directly model Acquire and
Release semantics.

## GCC

### Fences

Simple compiler fences can be expressed using an empty assembly directive:

``` c
__asm__ __volatile__("" ::: "memory");
```

## Java

### Fences

Java's `Unsafe` class provides access to the following fences:

  - loadFence
    equivalent to Acquire;
  - storeFence
    equivalent to Release;
  - fullFence
    equivalent to Seqcst.

### Volatile

The Java Memory Model (JMM) defines a volatile load as an Acquire operation:

``` asm
mov r1 [y]          ; Load.
#LoadLoad+LoadStore ; Acquire.
```

A volatile store is a Release operation followed by a StoreLoad fence:

``` asm
#StoreStore+LoadStore ; Release.
mov [x] 1             ; Store.
#StoreLoad
```

The StoreLoad fence is required to establish sequential consistency, which ensures that a volatile
store cannot be reordered with a volatile load that follows it in program order:

``` asm
#StoreStore+LoadStore ; Release.
mov [x] 1             ; Store.
#StoreLoad
mov r1 [y]            ; Load.
#LoadLoad+LoadStore   ; Acquire.
```

Note that the StoreLoad fence could have instead been issued before each volatile read, but this is
likely to be less efficient in practice, because volatile variables are typically read more than
they are written.

The following `Unsafe` methods provide Release semantics without the overhead of a StoreLoad fence:

  - putOrderedObject
  - putOrderedInt
  - putOrderedLong

## Dekker’s Algorithm

Dekker's Algorithm is best understood by the interleavings shown below:

**Uncontended Case \#1**

| thread x      | thread y      |
| ------------- | ------------- |
| x\_wants = 1  |               |
| y\_wants == 0 |               |
| <Acquired>    |               |
|               | y\_wants = 1  |
|               | x\_wants == 1 |

**Uncontended Case \#2**

| thread x      | thread y      |
| ------------- | ------------- |
|               | y\_wants = 1  |
|               | x\_wants == 0 |
|               | <Acquired>    |
| x\_wants = 1  |               |
| y\_wants == 1 |               |

**Contended Case \#1**

| thread x      | thread y      |
| ------------- | ------------- |
| x\_wants = 1  |               |
|               | y\_wants = 1  |
| y\_wants == 1 |               |
|               | x\_wants == 1 |

**Contended Case \#2**

| thread x      | thread y      |
| ------------- | ------------- |
|               | y\_wants = 1  |
| x\_wants = 1  |               |
|               | x\_wants == 1 |
| y\_wants == 1 |               |

**Contended Case \#3**

| thread x      | thread y      |
| ------------- | ------------- |
| x\_wants = 1  |               |
|               | y\_wants = 1  |
|               | x\_wants == 1 |
| y\_wants == 1 |               |

**Contended Case \#4**

| thread x      | thread y      |
| ------------- | ------------- |
|               | y\_wants = 1  |
| x\_wants = 1  |               |
| y\_wants == 1 |               |
|               | x\_wants == 1 |
