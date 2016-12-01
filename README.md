# The Treasure Box Library 

[![Build Status](https://api.travis-ci.org/waruqi/tbox.svg)](https://travis-ci.org/waruqi/tbox) [![Build status](https://ci.appveyor.com/api/projects/status/n6q15h2978vmgkww?svg=true)](https://ci.appveyor.com/project/waruqi/tbox) [![codecov](https://codecov.io/gh/waruqi/tbox/branch/master/graph/badge.svg)](https://codecov.io/gh/waruqi/tbox) [![Join the chat at https://gitter.im/waruqi/tboox](https://badges.gitter.im/waruqi/tboox.svg)](https://gitter.im/waruqi/tboox?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![donate](http://tboox.org/static/img/donate.svg)](http://tboox.org/donation/)

## Introduction ([中文](#中文))

TBOX is a glib-like cross-platform C library that is simple to use yet powerful in nature.

The project focuses on making C development easier and provides many modules (.e.g stream, asio, regex, container, algorithm ...), 
so that any developer can quickly pick it up and enjoy the productivity boost when developing in C language.

It supports the following platforms:

- Windows
- Macosx
- Linux
- Android
- iOS

And it provides many compiling options using [xmake](http://www.xmake.io):

* Release: Disable debug information, assertion, memory checking and enable optimization.
* Debug: Enable debug information, assertion, memory checking and disable optimization.
* Smallest: Disable all extensional modules and enable space optimization.
* Micro: compiling micro library (~64K) for the embed system.

If you want to know more, please refer to:

* [Documents](https://github.com/waruqi/tbox/wiki/documents)
* [Github](https://github.com/waruqi/tbox)
* [HomePage](http://tboox.org)

## Features

#### The stream library

- Supports file, data, http and socket source
- Supports the stream filter for gzip, charset and...
- Implements stream transfer
- Implements the static buffer stream for parsing data
- Supports coroutine and implements asynchronous operation

#### The coroutine library 

- Provides high-performance coroutine switch(refer to [reports](http://tboox.org/2016/10/28/benchbox-coroutine/))
- Supports arm, arm64, x86, x86_64 ..
- Provides channel interfaces
- Provides semaphore and lock interfaces
- Supports io socket and stream operation in coroutine
- Provides some io servers (http ..) using coroutine

#### The database library

- Supports mysql and sqlite3 database and enumerates data using the iterator mode

#### The xml parser library

- Supports DOM and SAX mode and Supports xpath

#### The serialization and deserialization library

- Supports xml, json, bplist, xplist, binary formats

#### The memory library

- Implements some memory pools for optimizing memory
- Supports fast memory error detecting. it can detect the following types of bugs for the debug mode:
  - out-of-bounds accesses to heap and globals
  - use-after-free
  - double-free, invalid free
  - memory leaks

#### The container library

- Implements hash table, single list, double list, vector, stack, queue
  and min/max heap. Supports iterator mode for algorithm

#### The algorithm library

- Uses the iterator mode
- Implements find, binary find and reverse find algorithm
- Implements sort, bubble sort, quick sort, heap sort and insert sort algorithm
- Implements count, walk items, reverse walk items, for_all and rfor_all

#### The network library

- Implements dns(cached)
- Implements ssl(openssl, polarssl, mbedtls)
- Implements http
- Implements cookies
- Supports ipv4, ipv6
- Supports coroutine

#### The platform library

- Implements timer, fast and low precision timer
- Implements atomic and atomic64 operation
- Implements spinlock, mutex, event, semaphore, thread and thread pool 
- Implements file, socket operation
- Implements poller using epoll, poll, select, kqueue ...
- Implements switch context interfaces for coroutine

#### The charset library

- Supports utf8, utf16, gbk, gb2312, uc2 and uc4
- Supports big endian and little endian mode

#### The zip library

- Supports gzip, zlibraw, zlib formats using the zlib library if exists
- Implements lzsw, lz77 and rlc algorithm

#### The utils library

- Implements base32, base64 encoder and decoder
- Implements assert and trace output for the debug mode
- Implements bits operation for parsing u8, u16, u32, u64 data

#### The math library

- Implements random generator
- Implements fast fixed-point calculation, Supports 6-bits, 16-bits, 30-bits fixed-point number

#### The libc library

- Implements lightweight libc library interfaces, the interface name contains `tb_xxx` prefix for avoiding conflict
- Implements strixxx strrxxx wcsixxx wcsrxxx interface extension
- Optimizes some frequently-used interface, .e.g. memset, memcpy, strcpy ... 
- Implements `memset_u16`, `memset_u32`, `memset_u64` extension interfaces

#### The libm library

- Implements lightweight libm library interfaces, the interface name contains `tb_xxx` prefix for avoiding conflict
- Supports float and double type

#### The regex library

- Supports match and replace
- Supports global/multiline/caseless mode
- Uses pcre, pcre2 and posix regex modules

#### The hash library

- Implements crc32, adler32, md5 and sha1 hash algorithm
- Implements some string hash algorithms (.e.g bkdr, fnv32, fnv64, sdbm, djb2, rshash, aphash ...)
- Implements uuid generator

#### The asynchronous io library (deprecated)

- Please uses coroutine to implement asio programming

## Todolist

- Add server module
- Reconstruction xml module
- Add more algorithms and container
- Optimizes more libc interfaces 
- Implements more libm interfaces and not wrap it only

## Projects

Some projects using tbox:

* [gbox](https://github.com/waruqi/gbox)
* [vm86](https://github.com/waruqi/vm86)
* [xmake](http://www.xmake.io)
* [itrace](https://github.com/waruqi/itrace)
* [more](https://github.com/waruqi/tbox/wiki/tbox-projects)

## Build

Please install xmake first: [xmake](https://github.com/waruqi/xmake)

    # build for the host platform
    cd ./tbox
    xmake

    # build for the mingw platform
    cd ./tbox
    xmake f -p mingw --sdk=/home/mingwsdk 
    xmake
    
    # build for the iphoneos platform
    cd ./tbox
    xmake f -p iphoneos 
    xmake
    
    # build for the android platform
    cd ./tbox
    xmake f -p android --ndk=xxxxx
    xmake
    
    # build for the linux cross-platform
    cd ./tbox
    xmake f -p linux --sdk=/home/sdk # --toolchains=/home/sdk/bin
    xmake

    
## Example

    #include "tbox/tbox.h"

    int main(int argc, char** argv)
    {
        // init tbox
        if (!tb_init(tb_null, tb_null)) return 0;

        // trace
        tb_trace_i("hello tbox");

        // init vector
        tb_vector_ref_t vector = tb_vector_init(0, tb_element_cstr(tb_true));
        if (vector)
        {
            // insert item
            tb_vector_insert_tail(vector, "hello");
            tb_vector_insert_tail(vector, "tbox");

            // dump all items
            tb_for_all (tb_char_t const*, cstr, vector)
            {
                // trace
                tb_trace_i("%s", cstr);
            }

            // exit vector
            tb_vector_exit(vector);
        }

        // init stream
        tb_stream_ref_t stream = tb_stream_init_from_url("http://www.xxx.com/file.txt");
        if (stream)
        {
            // open stream
            if (tb_stream_open(stream))
            {
                // read line
                tb_long_t size = 0;
                tb_char_t line[TB_STREAM_BLOCK_MAXN];
                while ((size = tb_stream_bread_line(stream, line, sizeof(line))) >= 0)
                {
                    // trace
                    tb_trace_i("line: %s", line);
                }
            }

            // exit stream
            tb_stream_exit(stream);
        }

        // wait some time
        getchar();

        // exit tbox
        tb_exit();
        return 0;
    }

#### Contacts

* Email：[waruqi@gmail.com](mailto:waruqi@gmail.com)
* Homepage：[TBOOX Open Source Project](http://www.tboox.org/cn)
* Community：[TBOOX Open Source Community](http://www.tboox.org/forum)

<h1 id="中文"></h1>

## 简介

TBOX是一个用c语言实现的跨平台开发库。

针对各个平台，封装了统一的接口，简化了各类开发过程中常用操作，使你在开发过程中，更加关注实际应用的开发，而不是把时间浪费在琐碎的接口兼容性上面，并且充分利用了各个平台独有的一些特性进行优化。

这个项目的目的，是为了使C开发更加的简单高效。

目前支持的平台有：

- Windows
- Macosx
- Linux
- Android
- iOS

通过[xmake](http://www.xmake.io/cn)支持各种编译模式：

* Release: 正式版编译，禁用调试信息、断言，各种检测机制，启用编译器优化
* Debug: 调试模式，默认启用详细调试信息、断言、内存越界检测、内存泄漏、锁竞争分析等检测机制
* Smallest: 最小化编译，默认禁用所有扩展模块，启用编译器最小化优化
* Micro: 针对嵌入式平台，仅仅编译tbox微内核，仅提供最基础的跨平台接口（内置轻量libc接口实现）

如果你想了解更多，请参考：

* [在线文档](https://github.com/waruqi/tbox/wiki/%E7%9B%AE%E5%BD%95)
* [在线源码](https://github.com/waruqi/tbox)
* [项目主页](http://www.tboox.org/cn)

## 特性

#### 流库

针对http、file、socket、data等流数据，实现统一接口进行读写，并且支持: 阻塞、非阻塞、异步 三种读写模式。
支持中间增加多层filter流进行流过滤，实现边读取，内部边进行解压、编码转换、加密等操作，极大的减少了内存使用。 

主要提供以下模块：

- `stream`：通用非阻塞流，用于一般的单独io处理，同时支持协程以实现异步传输。
- `transfer`：流传输器，维护两路流的传输。
- `static_stream`：针对静态数据buffer优化的静态流，用于轻量快速的数据解析。

#### 协程库

- 快速高效的协程切换支持（具体性能参考：[基准测试报告](http://tboox.org/cn/2016/10/28/benchbox-coroutine/)）
- 提供跨平台支持，核心切换算法参考boost，并且对其进行重写和优化，目前支持架构：x86, x86_64, arm, arm64
- 提供channel协程间数据通信支持，基于生产、消费者模型
- 提供信号量、协程锁支持
- socket、stream都模块原生支持协程，并且可在线程和协程间进行无缝切换
- 提供http、file等基于协程的简单服务器实例，只需几百行代码，就可以从socket开始写个高性能io服务器，代码逻辑比异步回调模式更加清晰

#### 数据库

- 统一并简化数据库操作接口，适配各种数据源，通过统一的url来自动连接打开支持的数据库，数据的枚举采用迭代器模型。
- 目前支持sqlite3以及mysql两种关系型数据库，也可自定义扩展使用其他关系型数据库。

#### xml库

- 针对xml提供DOM和SAX两种解析模式，SAX方式采用外部迭代模式，灵活性和性能更高，并且可以选择指定路径，进行解析。
- 解析过程完全基于stream，所以是高度流化的，可以实现边下载、边解压、边转码、边解析一条龙服务，使用较低的内存也可以解析大规模数据。
- 提供xml writer以支持对xml生成

#### 内存库

- 参考linux内核内存管理机制的实现，并对其进行各种改造和优化，所实现的TBOX独有的一整套内存池管理架构。
- 调试模式下，可以轻松检测并定位内存泄露、内存越界溢出、内存重叠覆盖等常见内存问题，并对整体内存的使用进行了统计和简要分析。
- 针对大块数据、小块数据、字符串数据进行了充分的利用，避免了大量外部碎片和内部碎片的产生。分配操作进行了各种优化，96%的情况下，效率都是在O(1)。

#### 容器库

- 提供哈希、链表、数组、队列、堆栈、最小最大堆等常用容器。
- 支持各种常用成员类型，在原有的容器期初上，其成员类型还可以完全自定义扩展。
- 所有容器都支持迭代器操作。
- 大部分容器都可以支持基于stream的序列化和反序列化操作。

#### 算法库

- 提供各种排序算法：冒泡排序、堆排序、快速排序、插入排序。
- 提供各种查找算法：线性遍历、二分法搜索。
- 提供各种遍历、删除、统计算法。
- 以迭代器为接口，实现算法和容器的分离，类似stl，但是c实现的，更加轻量。

#### 网络库

- 实现http客户端模块
- 实现cookies
- 实现dns解析与缓存
- 实现ssl(支持openssl, polarssl, mbedtls)
- 支持ipv4、ipv6
- 支持通过协程实现异步模式

#### 数学运算库

- 提供各种精度的定点运算支持
- 提供随机数生成器

#### libc库

- libc的一个轻量级实现，完全跨平台，并且针对不同架构进行了优化。
- 支持大部分字符串、宽字符串操作。
- 扩展字符串、宽字符串的各种大小写不敏感操作接口
- 扩展`memset_u16`、`memset_u32`等接口，并对其进行高度优化，尤其适合图形渲染程序

#### libm库

- libm部分接口的一个轻量级实现，以及对常用系统接口的封装。（目前只实现了部分，之后有时间会完全实现掉）
- 扩展部分常用接口，增加对sqrt、log2等常用函数的整数版本计算，进行高度优化，不涉及浮点运算，适合嵌入式环境使用。

#### object库

- 轻量级类apple的CoreFoundation库，支持object、dictionary、array、string、number、date、data等常用对象，并且可以方便扩展自定义对象的序列化。
- 支持对xml、json、binary以及apple的plist(xplist/bplist)格式序列化和反序列化。
并且实现自有的binary序列化格式， 针对明文进行了简单的加密，在不影响性能的前提下，序列化后的大小比bplist节省30%。

#### 平台库

- 提供file、directory、socket、thread、time等常用系统接口
- 提供atomic、atomic64接口
- 提供高精度、低精度定时器
- 提供高性能的线程池操作
- 提供event、mutex、semaphore、spinlock等事件、互斥、信号量、自旋锁操作
- 提供获取函数堆栈信息的接口，方便调试和错误定位
- 提供跨平台动态库加载接口（如果系统支持的话）
- 提供io轮询器，针对epoll, poll, select, kqueue进行跨平台封装
- 提供跨平台上下文切换接口，主要用于协程实现，切换效率非常高

#### 压缩库

- 支持zlib/zlibraw/gzip的压缩与解压（需要第三方zlib库支持）。

#### 字符编码库

- 支持utf8、utf16、gbk、gb2312、uc2、uc4 之间的互相转码，并且支持大小端格式。

#### 实用工具库

- 实现base64/32编解码
- 实现crc32、adler32、md5、sha1等常用hash算法
- 实现日志输出、断言等辅助调试工具
- 实现url编解码
- 实现位操作相关接口，支持各种数据格式的解析，可以对8bits、16bits、32bits、64bits、float、double以及任意bits的字段进行解析操作，并且同时支持大端、小端和本地端模式，并针对部分操作进行了优化，像static_stream、stream都有相关接口对其进行了封装，方便在流上进行快速数据解析。
- 实现swap16、swap32、swap64等位交换操作，并针对各个平台进行了优化。
- 实现一些高级的位处理接口，例如：位0的快速统计、前导0和前导1的快速位计数、后导01的快速位计数
- 实现单例模块，可以对静态对象、实例对象进行快速的单例封装，实现全局线程安全
- 实现option模块，对命令行参数进行解析，提供快速方便的命令行选项建立和解析操作，对于写终端程序还是很有帮助的   

#### 正则表达式库

- 支持匹配和替换操作
- 支持全局、多行、大小写不敏感等模式
- 使用pcre, pcre2和posix正则库

#### asio库 (已废弃)

- 请使用协程来进行异步io编程

## 后续任务

- 重构xml模块
- 添加更多的容器和算法
- 优化更多的libc接口
- 实现更多的libm接口，而不仅仅是包裹到系统api

## 一些使用tbox的项目：

* [gbox](https://github.com/waruqi/gbox)
* [vm86](https://github.com/waruqi/vm86)
* [xmake](http://www.xmake.io/cn)
* [itrace](https://github.com/waruqi/itrace)
* [更多项目](https://github.com/waruqi/tbox/wiki/%E4%BD%BF%E7%94%A8tbox%E7%9A%84%E5%BC%80%E6%BA%90%E5%BA%93)

## 编译 

请先安装: [xmake](https://github.com/waruqi/xmake)

    # 默认直接编译当前主机平台
    cd ./tbox
    xmake

    # 编译mingw平台
    cd ./tbox
    xmake f -p mingw --sdk=/home/mingwsdk 
    xmake

    # 编译iphoneos平台
    cd ./tbox
    xmake f -p iphoneos 
    xmake
    
    # 编译android平台
    cd ./tbox
    xmake f -p android --ndk=xxxxx
    xmake
    
    # 交叉编译
    cd ./tbox
    xmake f -p linux --sdk=/home/sdk #--toolchains=/home/sdk/bin
    xmake


## 例子

    #include "tbox/tbox.h"

    int main(int argc, char** argv)
    {
        // init tbox
        if (!tb_init(tb_null, tb_null)) return 0;

        // trace
        tb_trace_i("hello tbox");

        // init vector
        tb_vector_ref_t vector = tb_vector_init(0, tb_element_cstr(tb_true));
        if (vector)
        {
            // insert item
            tb_vector_insert_tail(vector, "hello");
            tb_vector_insert_tail(vector, "tbox");

            // dump all items
            tb_for_all (tb_char_t const*, cstr, vector)
            {
                // trace
                tb_trace_i("%s", cstr);
            }

            // exit vector
            tb_vector_exit(vector);
        }

        // init stream
        tb_stream_ref_t stream = tb_stream_init_from_url("http://www.xxx.com/file.txt");
        if (stream)
        {
            // open stream
            if (tb_stream_open(stream))
            {
                // read line
                tb_long_t size = 0;
                tb_char_t line[TB_STREAM_BLOCK_MAXN];
                while ((size = tb_stream_bread_line(stream, line, sizeof(line))) >= 0)
                {
                    // trace
                    tb_trace_i("line: %s", line);
                }
            }

            // exit stream
            tb_stream_exit(stream);
        }

        // wait some time
        getchar();

        // exit tbox
        tb_exit();
        return 0;
    }

#### 联系方式

* 邮箱：[waruqi@gmail.com](mailto:waruqi@gmail.com)
* 主页：[TBOOX开源工程](http://www.tboox.org/cn)
* 社区：[TBOOX开源社区](http://www.tboox.org/forum)
* QQ群：343118190
* 微信公众号：tboox-os

