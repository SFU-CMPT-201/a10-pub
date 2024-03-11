# Assignment 10: Simple MapReduce

In this assignment, you will implement a simple version of
[MapReduce](https://research.google/pubs/mapreduce-simplified-data-processing-on-large-clusters/).
MapReduce is a programming framework for processing large data sets with a parallel, distributed
algorithm on a cluster. It was inspired by the
[map](https://en.wikipedia.org/wiki/Map_(higher-order_function)) and
[reduce](https://en.wikipedia.org/wiki/Fold_(higher-order_function)) functions commonly used in
functional programming. The key components of the framework are those two functions: `map` and
`reduce`. The `map` function processes a key-value pair to generate a set of intermediate key-value
pairs. The `reduce` function processes the intermediate key-value pairs to generate the final
output. The framework is widely used in distributed computing, and it is the foundation of many big
data processing systems, such as [Hadoop](https://hadoop.apache.org/) and
[Spark](https://spark.apache.org/). It is used in many courses as assignments due to its elegance
and popularity. In this assignment, you will implement a simple version of the MapReduce framework
using threads and synchronization primitives.

We note that the description here largely comes from [the original MapReduce
paper](https://storage.googleapis.com/gweb-research2023-media/pubtools/pdf/16cb30b4b92fd4989b8619a61752a2387c6dd474.pdf).

## Understanding the Basics of MapReduce

MapReduce takes a list of key-value pairs as input and produces a list of new key-value pairs as
output. A key-value pair is a tuple where the first element is called the key and the second element
is called the value. For example, `("name", "John")` is a key-value pair where `"name"` is the key
and `"John"` is the value. MapReduce takes a list of such key-value pairs as input and processes
them to generate a list of new key-value pairs as output. As a side note, if you have many key-value
pairs, you can think of them as a table with two columns, where the first column is the key and the
second column is the value. A key can be used to look up the corresponding value in the table.

For example, suppose we have the following list of key-value pairs as input, where the key is a line
number and the value is a line of text:

```
("1", "the quick brown fox")
("2", "jumps over the lazy dog")
("3", "the quick brown fox")
("4", "jumps over the lazy dog")
```

Or, in table form:

```
+-----+-------------------------+
| key | value                   |
+-----+-------------------------+
| 1   | the quick brown fox     |
| 2   | jumps over the lazy dog |
| 3   | the quick brown fox     |
| 4   | jumps over the lazy dog |
+-----+-------------------------+
```

Since MapReduce does not use this table form, we will not use it in the rest of this document.

Now, suppose we use MapReduce to process the above list of key-value pairs to count the number of
occurrences of each word. The output could look like the following, where the key is a word and the
value is the number of occurrences of the word in the input:

```
("the", "4")
("quick", "2")
("brown", "2")
("fox", "2")
("jumps", "2")
("over", "2")
("lazy", "2")
("dog", "2")
```

Another side note here is that using this output, we can use the word as the key to look up the
number of occurrences of the word.

If a developer wants to use MapReduce to perform this kind of processing, they need to write two
functions, `map` and `reduce`. The MapReduce framework takes those functions, runs them, and
performs all the necessary work to produce the final output. We will look at the details shortly,
but in general, the `map` function (written by the developer) takes a key-value pair as input and
generates a list of *intermediate* key-value pairs as output. The MapReduce framework then collects
*all* the intermediate values associated with *each* intermediate key and passes them to the
`reduce` function. The `reduce` function (also written by the developer) receives an intermediate
key and the corresponding list of intermediate values as input and generates a list of new key-value
pairs as output.

Let's apply this to the example above. A developer needs to write the `map` and `reduce` functions
to count the number of occurrences of each word. Thus, the developer writes the `map` function to
count the number of occurrences of each word in *each line* of input. Then the developer writes the
`reduce` function to *sum up* the numbers from each line to get the total number of occurrences of
each word.

More specifically, the developer writes the `map` function so that it takes one input key-value pair
(e.g., `("1", "the quick brown fox")`) and, for each word, generates an intermediate key-value pair
where the key is the word and the value is the number 1 (e.g., `("the", "1")`, `("quick", "1")`,
`("brown", "1")`, `("fox", "1")`). The number 1 indicates that the word has occurred once. The
following shows the complete intermediate key-value pairs produced by such a `map` function (`map`
is called four times, once for each line of input):

```
("the", "1")
("quick", "1")
("brown", "1")
("fox", "1")
("jumps", "1")
("over", "1")
("the", "1")
("lazy", "1")
("dog", "1")
("the", "1")
("quick", "1")
("brown", "1")
("fox", "1")
("jumps", "1")
("over", "1")
("the", "1")
("lazy", "1")
("dog", "1")
```

Then the MapReduce framework (i.e., *not* the developer) collects all the intermediate key-value
pairs produced by the above `map` function, and performs *grouping*. What this grouping does is
that, for each intermediate key, it collects all the intermediate values associated with the key and
makes a list of them. The output looks like the following, where the key is the word and the value
is a list of all intermediate values (i.e., 1s) associated with each key (i.e., each word):

```
("the", ["1", "1", "1", "1"])
("quick", ["1", "1"])
("brown", ["1", "1"])
("fox", ["1", "1"])
("jumps", ["1", "1"])
("over", ["1", "1"])
("lazy", ["1", "1"])
("dog", ["1", "1"])
```

The MapReduce framework then passes each intermediate key and its associated list of intermediate
values to the `reduce` function. The `reduce` function (written by the developer) then sums up the
numbers in the list to get the total number of occurrences of each word. The following shows the
complete processing of all the intermediate key-value pairs with such a `reduce` function (`reduce`
is called eight times, once for each intermediate key-value pair):

```
("the", "4")
("quick", "2")
("brown", "2")
("fox", "2")
("jumps", "2")
("over", "2")
("lazy", "2")
("dog", "2")
```

The above final output is the list of key-value pairs that show the number of occurrences of each
word in the input.

As it turns out, `map` and `reduce` are quite powerful and can be used to solve a wide variety of
problems. For example, `map` and `reduce` can be used to find a line in a file that contains a word,
to count the access frequency of a web page in a web server log, to get the list of all URLs that
contain a link to a specific web page, and so on.

## Understanding Concurrency in MapReduce

The original MapReduce is designed to process large data sets where the `map` and `reduce` functions
are executed concurrently on a cluster of machines. In this assignment, you will not use a cluster
of machines, but you will use threads to execute `map` and `reduce` concurrently on a single
machine.

The concurrent execution of `map` and `reduce` works as follows.
* The MapReduce framework creates a set of threads to execute the `map` function and another set of
  threads to execute the `reduce` function.
* The framework *partitions* the input key-value pairs into multiple chunks. The number of chunks is
  equal to the number of `map` threads. Let's consider the above example again to understand this
  better. If we have two `map` threads, the framework partitions the input key-value pairs into two
  chunks. The following will be the partitioning of the input key-value pairs:
    * Chunk 0: `("1", "the quick brown fox"), ("2", "jumps over the lazy dog")`
    * Chunk 1: `("3", "the quick brown fox"), ("4", "jumps over the lazy dog")`
* The framework then assigns each chunk to a `map` thread.
* Each `map` thread processes its assigned chunk by calling the `map` function repeatedly, once for
  each input key-value pair in the assigned chunk. Note that all the `map` threads run at the same
  time, processing their assigned chunks concurrently.
* The framework then collects all the intermediate key-value pairs produced by all `map` threads.
* The framework then again *partitions* the intermediate key-value pairs into multiple chunks and
  assigns each chunk to a `reduce` thread. Again using the above example, if we have four `reduce`
  threads, the framework partitions the intermediate key-value pairs into four chunks. The following
  will be the partitioning of the intermediate key-value pairs:
    * Chunk 0: `("the", ["1", "1", "1", "1"]), ("quick", ["1", "1"])`
    * Chunk 1: `("brown", ["1", "1"]), ("fox", ["1", "1"])`
    * Chunk 2: `("jumps", ["1", "1"]), ("over", ["1", "1"])`
    * Chunk 3: `("lazy", ["1", "1"]), ("dog", ["1", "1"])`
* The framework then assigns each chunk to a `reduce` thread.
* Each `reduce` thread processes its assigned chunk by calling the `reduce` function repeatedly,
  once for each intermediate key and its associated list of intermediate values in its assigned
  chunk. Note that all the `reduce` threads run at the same time, processing their assigned chunks
  concurrently.
* The framework then collects all the final key-value pairs produced by all `reduce` threads and
  outputs them as the final output.

## Requirements

There are several requirements for this assignment.
* Your MapReduce framework is a library that implements necessary functions that a developer uses
  to process their data.
* [interface.h](./include/interface.h) defines the functions that you need to implement.
    * `mr_exec()` executes the MapReduce framework. A developer calls this function to run
      `map()` and `reduce()`, and get the final output. You need to write this function to run the
      framework. A developer can call this function as many times as they want to process different
      input key-value pairs. The function takes the following arguments.
        * The input key-value pairs.
        * The `map()` and `reduce()` functions that the framework should execute. Note that a
          developer writes `map()` and `reduce()` functions. `map()` takes *one* key-value pair as
          input and generates *zero or more* new intermediate key-value pairs as output. `reduce()`
          takes *one* intermediate key and the list of intermediate values associated with the key
          as input and generates *zero or more* new final key-value pairs as output.
        * The number of `map` threads and the number of `reduce` threads. You need to create the
          specified number of threads to execute the `map` and `reduce` functions.
        * A buffer for storing the final output. The framework should store the final output in this
          buffer.
    * `mr_emit_i()` and `mr_emit_f()` handle intermediate and final key-value pairs. A developer
      calls (within `map`) `mr_emit_i()` to pass intermediate key-value pairs to the framework, and
      calls (within `reduce`) `mr_emit_f()` to pass final key-value pairs to the framework. You need
      to write these two functions to collect intermediate and final key-value pairs. A developer
      can call these functions as many times as they want within `map()` and `reduce()`,
      respectively.
    * The order of the input key-value pairs should be preserved for `map()`. For example, if the
      input key-value pairs have the following order: `("1", "the quick brown fox")`, `("2", "jumps
      over the lazy dog")`, `("3", "the quick brown fox")`, and `("4", "jumps over the lazy dog")`,
      the framework should call `map()` with the input key-value pairs in the same order. When
      dividing the input into chunks, the framework should also preserve the order of the input
      key-value pairs. For example, if we use the above input key-value pairs and two `map` threads,
      the framework should call `map()` with the following two chunks: `("1", "the quick brown
      fox"), ("2", "jumps over the lazy dog")` and `("3", "the quick brown fox"), ("4", "jumps over
      the lazy dog")`, one for each thread.
    * The order of the intermediate key-value pairs should be sorted by the key, according to the
      result of the `strcmp()` function. For example, if the intermediate key-value pairs have the
      following key-value pairs: `("brown", "1")`, `("fox", "1")`, `("dog", "1")`, `("brown", "1")`,
      and `("fox", "1")`, the framework should sort the intermediate key-value pairs in the
      following order: `("brown", "1")`, `("brown", "1")`, `("dog", "1")`, `("fox", "1")`, and
      `("fox", "1")`. The framework should then group them and divide them into chunks in the same
      way as the input key-value pairs.
    * Similarly, the order of the output key-value pairs should be sorted by the key, according to
      the result of the `strcmp()` function. For example, if the final output has the following
      key-value pairs: `("brown", "2")`, `("fox", "2")`, and `("dog", "2")`, the framework should
      store the final output in the following order: `("brown", "2")`, `("dog", "2")`, and `("fox",
      "2")`.
    * When partitioning input or intermediate key-value pairs into chunks, the size of each chunk
      should be the size of `(all key-value pairs) / (number of threads)`, except for the last
      chunk, which should contain the remaining key-value pairs if there's any. For example, if we
      have 7 key-value pairs and 3 threads, the first two chunks should contain 3 key-value pairs
      each, and the last chunk should contain 1 key-value pairs. If we have 6 key-value pairs
      instead, each chunk should contain 2 key-value pairs.
* You need to use `pthread` to create threads. You should also use appropriate synchronization
  primitives to ensure that the threads work correctly.
* You need to write `CMakeLists.txt` that produces a single executable named `mapreduce` that runs
  the test cases in [`main.c`](./src/main.c).
* You may find [uthash](https://troydhanson.github.io/uthash/userguide.html) useful.
* Similar to previous assignments, you should not hard-code or customize your implementation
  tailored to our test cases. Generally, you should consider the provided test cases as examples or
  specific instances of general cases. For example, we can change the constants, the input key-value
  pairs, the `map` and `reduce` functions, the number of `map` and `reduce` threads, and so on. Your
  implementation should still work correctly.

## Grading Distribution

* A single map thread and a single reduce thread
    * [10 pts] Correctly call map with the input key-value pairs using a single thread.
    * [10 pts] Correctly call reduce with the intermediate key-value pairs using a single thread.
    * [15 pts] Correctly generate final output using a single map thread and a single reduce thread.
* Multiple `map` and `reduce` threads
    * [5 pts] Run the correct number of `map` threads.
    * [5 pts] Run the correct number of `reduce` threads.
    * [15 pts] Correctly partition the input key-value pairs into multiple chunks and assign each
      chunk to a `map` thread.
    * [15 pts] Correctly partition the intermediate key-value pairs into multiple chunks and assign
      each chunk to a `reduce` thread.
    * [15 pts] Correctly generate final output.
* [5 pts] Pass all the above test cases.
* [5 pts] `CMake` builds correctly.
* Code that does not compile gets a 0.
* You should not have any data races. We will test this with Clang's thread sanitizer. If a data
  race is found, there will be a penalty of -10 pts.
* A deadlock or a livelock should not occur. If a deadlock or a livelock occurs at any point, the
  grader will stop grading the code and you will get whatever points you have at that time.
* You should not have any memory issues. We will test this with Clang's address sanitizer. If a
  memory issue is found, there will be a penalty of -10 pts.
* You need to use the same code structure as A8. A wrong code directory structure has a penalty of
  -10 pts.
