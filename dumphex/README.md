# dumphex

try write my own version of `hexdump`, but more like `xxd`.

1. ~just print all the bytes out~
1. ~print printable characters out alongside the hex~
1. ~add colors like xxd~

it works!

## links/ref:

- found the source of the `xxd` that I have: <https://github.com/vim/vim/blob/master/src/xxd/xxd.c> for reference.
- color control codes: <https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124>
- good ol ascii table: <https://condor.depaul.edu/sjost/lsp121/documents/ascii-npr.htm>

## build and run

```
cc -o dumphex main.c
./dumphex dumphex # look at the bytes of dumphex
./dumphex main.c
```

## other ideas

1. be able to choose the column width with args
1. support a flag that just dumps the bytes as hex with no other decorations

