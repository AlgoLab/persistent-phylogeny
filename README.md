# Persistent Phylogeny

An implementation for the persistent phylogeny polynomial-time algorithm described the article [Solving the Persistent Phylogeny Problem in polynomial time](https://arxiv.org/abs/1611.01017).

## Prerequisites

- gcc 5.0 and above
- [Boost libraries](http://www.boost.org/more/getting_started/index.html)

## Compiling

### Get Boost

> N.B. Skip this block if you already have Boost on your system

#### From repository

Popular Linux and Unix distributions such as Fedora, Debian, and NetBSD include pre-built Boost packages.

Debian example:

```
# apt install libboost-all-dev
```

#### From source

Download Boost.

- Unix: follow the steps at [boost.org](http://www.boost.org/doc/libs/1_65_1/more/getting_started/unix-variants.html)
- Windows: follow the steps at [boost.org](http://www.boost.org/doc/libs/1_65_1/more/getting_started/windows.html)

##### Boost.ProgramOptions

> N.B. Skip this step if you already built Boost on your system

This implementation uses `Boost.ProgramOptions` to parse command line arguments and options, which means it needs separately-compiled library binaries to work.

- Unix: follow the steps at [boost.org](http://www.boost.org/doc/libs/1_65_1/more/getting_started/unix-variants.html#prepare-to-use-a-boost-library-binary)
- Windows: follow the steps at [boost.org](http://www.boost.org/doc/libs/1_65_1/more/getting_started/windows.html#prepare-to-use-a-boost-library-binary)

> NB. Skip this step if you already updated your platform's environment variable

Moreover, on Unix systems, you may need to prepare plaform-specific settings (since `Boost.ProgramOptions` is a shared library).

- Follow the steps at [boost.org](http://www.boost.org/doc/libs/1_65_1/more/getting_started/unix-variants.html#test-your-program)

### Clone the repository

```
$ git clone https://github.com/vezzo95/persistent-phylogeny.git
$ cd persistent-phylogeny
```

### Compile and link

```
$ make
```

## Usage

```
ppp [OPTION...] FILE...
```

**ppp** corresponds to the executable binary.

**OPTION** is a list of options - can be omitted.

**FILE** is a list of file paths (1 or more).

### Options

```
-h or  --help
```

Display this message.

___

```
-v or --verbose
```

Display the operations performed by the program.

___

```
-t or --testpy
```

Test the output of the algorithm with check_reduction.py.
This can be used to make sure the output of the program is correct.
**Don't** use this option when benchmarking; the Python script takes time to complete.

___

```
-x or --exponential
```

Exponential version of the algorithm.  
This option can be used to test every possible combination of safe sources.  
It is also mutually exclusive with `--interactive` and `--nthsource`.

___

```
-i or --interactive
```

User input driven execution.  
This option can be used to manually select which safe source to realize.  
It is also mutually exclusive with `--exponential` and `--nthsource`.

___

```
-m or --maximal
```

Run the algorithm on the maximal subgraph instead of the full graph.  
This is done by calculating a maximal reducible graph before running the algorithm.

___

```
-n N or --nthsource N
```

Select the nth safe source when possible (default 0 - the option can be omitted).  
This option can be used to automatically select the nth safe source to realize (instead of manually selecting it each time with `--interactive`).  
It is also mutually exclusive with `--exponential` and `--interactive`.

___

```
-a or --active
```
Use this option if you want to include active characters when building the hasse diagram.

___

## Running

```
$ ./bin/ppp FILE...
```

The program will call the `reduce` function on each graph read from the files.

Examples:

Listing the single files

```
$ ./bin/ppp file1 dir1/file2 dir1/file3
```

Or with globs (if your shell supports it)

```
$ ./bin/ppp dir1/*
```

Or with options (order doesn't matter - still supports globs)

```
$ ./bin/ppp -m -v file1
```

## Input file structure

The first line must contain the size of the matrix and the list of characters to be set as active. 
Empty lines (after the first) are ignored.  
Subsequent lines represent the matrix itself.

```
N M c0 c1 c2... ci...  

Matrix made up of space-separated boolean values
```

Example which has no active characters by default:

```
6 3

0 0 1
0 1 0
0 1 1
1 0 0
1 0 1
1 1 0
```

Example which has the characters c0 and c2 active:

```
6 3 0 2

0 0 1
0 1 0
0 1 1
1 0 0
1 0 1
1 1 0
```
