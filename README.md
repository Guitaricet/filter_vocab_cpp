# Text Filter
Simple and fast vocabulary-based text filtration in C++

## Installaiton
```
pip install pybind11
make
```

## `text_filter` function

```
def text_filter(text, vocab, min_words, p_max_oov, from_sentence_start_only):
    """
    Filter vocabulary based on given parameters.

    Parameters
    ----------
    text : str
        The input text to be filtered.
    vocab : set[str]
        The vocabulary set.
    min_words : int
        The minimum number of words to consider for a chunk.
    p_max_oov : float
        The maximum out-of-vocabulary word percentage allowed in a chunk.
    from_sentence_start_only : bool
        Whether to only start chunks at the start of sentences.

    Returns
    -------
    list[tuple[str, FilteringInfo]]
        List of tuples where each tuple consists of the chunk string and its associated FilteringInfo.
```

## Usage
Treat the compiled file that will be called something like `text_filter.cpython-310-x86_64-linux-gnu.so` like a python module named `text_filter`.

```
import text_filter

text = "This is a sample text. Some words are out of vocabulary like extraterrestrial and martian."
vocab = set(["this", "is", "a", "sample", "text", ".", "some", "words", "are", "out", "of", "vocabulary", "like"])

print(text_filter.tokenize(text))
chunks = text_filter.filter_vocab(text, vocab, 3, 0.2, True)
chunks
```

will output

```
[('This is a sample text. Some words are out of vocabulary like extraterrestrial',
  FilteringInfo(word_count=14, oov_count=2))]
```

More examples can be found in `test.ipynb`.

## Known Issues

Does not work on Apple Silicon (M1/M2) Macs
