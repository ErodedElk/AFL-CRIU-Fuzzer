# AFL-CRIU-Fuzzer

基于快照实现的一个 Fuzzer 样品，目前尚且不能正常工作。

想要实现这样一个功能：

> 进程正常启动后，在想要 fuzz 的目标函数处暂停。此时 dump 出进程快照交给 AFL。
> AFL 将会恢复快照，并将 testcase 注入内存，并重启进程。此后交给 fork server 继续工作。

# 原理

基于 CRIU 提供进程快照功能。在目标函数处注入 `jmp $` 指令，然后使用 CRIU 获得进程快照。

在 CRIU 恢复进程快照时将 `jmp $` 重新 patch 为 `nop` 即可重启进程。

对于 AFL ，将原本的 `execv` 用唤醒进程的函数替代。同时另外将 testcase 注入到寄存器/内存中。

> 因为测试目标均为函数，所以需要注入的数据为参数，可以通过注入寄存器/内存的方式 hook 参数。

目标进程仍然使用 fork server 来避免开销。

# 尚未解决的问题

目前 AFL 还不能正常工作。目标进程被恢复以后并不属于子进程，其文件描述符不会从 AFL 处继承，导致 AFL 无法和 fork server 通信。

具体还需要进一步了解 CRIU 的实现机制和代码才能进一步改进。

# 如何编译

先编译 AFL，然后再编译 CRIU 即可：

```
cd AFL
make
cd ..
make
```

将会在 criu/criu 目录下生成可执行文件 `criu`

