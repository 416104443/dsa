dsa
===

## description

dsa is a utility library of data structures and algorithms for C++11 and later.

All of the components are written in the style of the (modern, i.e. C++11 and
onwards) C++ Standard Template Library, and are intentionally designed to be
fully compatible with the data structures and algorithms provided therein. All
code is written to be portable across platforms so long as they support the
C++11 standard.

Future goals include parallelization extensions, where appropriate for each
provided data structure and algorithm, in the style of the C++17 [Library
Extensions for Parallelism]
(http://en.cppreference.com/w/cpp/experimental/parallelism).

## dependencies

Compiler support for C++11 or later.

## index

* [bstree](./bstree)
* [dynamic_ringbuffer](./dynamic_ringbuffer)
* [ringbuffer](./ringbuffer)
* [sorting](./sorting)
* [spinlock](./spinlock)

## info

### author

Dalton Woodard

### contact

daltonmwoodard@gmail.com

### official repository

https://github.com/daltonwoodard/dsa.git

### License

All of the following information is reproduced in [COPYRIGHT](COPYRIGHT.txt).

The dsa library and all of its component repositories are distributed under a
dual MIT License and Apache-2.0 License. You, the licensee, may choose either at
your option. The MIT License is GPL compatible, while the Apache-2.0 License is
not, so please take this into consideration.

Also note that by choosing *either* the MIT license *or* Apache-2.0 license
you are required to provide attribution in the form of copyright reproduction
(the details of which are subject to the license you choose).

The terms of each can be found in the files [LICENSE-MIT](LICENSE-MIT) and
[LICENSE-APACHE-2.0](LICENSE-APACHE-2.0), respectively. The notices of each are
reproduced here for convenience:

---

MIT License (MIT)

Copyright (c) 2016 Dalton M. Woodard

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

Copyright 2016 Dalton M. Woodard

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
