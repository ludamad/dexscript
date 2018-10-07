dexscript
=========

An interpreter for DexScript, a language that aims to be Python without side-effects (in the functional sense). Can be compiled with CMake.

Provides a complete but primitive dynamic programming language with entirely immutable objects, with a Copy-On-Write (COW) performance optimization for objects. That is, you can treat objects as entirely immutable, but so long as you edit a single object an object can be modified in-place. A special `in` keyword for variables exists to signify they should be updated with the result of a function call. This aids in propagating efficient object mutability down a chain of functions, without introducing true mutable references.

For a fairly complete example of the language, see https://github.com/ludamad/dexscript/blob/master/bin/bf_interpreter.dex.
