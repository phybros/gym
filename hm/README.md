# hm: how many

naming is hard.

try to make my own version of `wc`

`wc` is apparently "word, line, character, and byte count". i thought it was just for counting lines but i guess i was wrong.

ideas

1. surely just counting the number of `\n` can't be it...
1. but i'll try that first

it works! at least in the simple case

```bash
$ ./hm main.c
38 main.c
$ wc -l main.c
38 main.c
$ cat main.c | ./hm
38
$ cat main.c | wc -l
38
```

need to figure out the best way to accept flags so i can also count words, bytes, and characters

