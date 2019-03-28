# yskip

C++ implementation of incremental skip-gram model with negative sampling [(Kaji and Kobayashi, 2017)](http://aclweb.org/anthology/D17-1037).

## Installation

```
% ./configure
% make
% make install
```

gcc 4.4.7 or higher is required.


## Basic Usage

The following command learns skip-gram model from the plain text file `text` and save it as `model`:
```   
% yskip text model
```

### Learning from the standard input stream

`yskip` is able to learn from the standard input stream (without allocating the entire input on memory) by setting the input file name to `-`:
```   
% cat text | yskip - model
```
This is usable only in online and mini-batch setting (see the options).


## Options

`yskip` supports learning options listed below.

```
yskip [option] <train> <model>

Skip-gram model paramters:
 -d, --dimensionality-size=INT      Dimensionality of word embeddings (default: 100)
 -w, --window-size=INT              Window size (default: 5)
 -n, --negative-sample=INT          Number of negative samples (default: 5)
 -a, --alpha=FLOAT                  Distortion parameter (default: 0.75)
 -s, --subsampling-threshold=FLOAT  Subsampling threshold (default: 1.0e-3)
 -u, --unigram-table-size=INT       Unigram table size used for negative sampling (default: 1e8)
 -m, --max-vocabulary-size=INT      Maximum vocabulary size (default: 1e6)
 -e, --eta=FLOAT                    Initial learning rate of AdaGrad (default: 0.1)
 -b, --mini-batch-size=INT          Mini-batch size (default: 10000)
 -B, --binary-mode                  Read/write models in a binary format
 -l, --learning-strategy=INT        Learning strategy
                                    0: batch
                                    1: online
                                    2: mini-batch (default)
 -i, --iteration-numbedr            iteration number in batch learning (default: 5)

Misc.:
 -t, --thread-num=INT               Number of threads (default: 10)
 -I, --initial-model=FILE           Initial model (default: NULL)
 -r, --random-seed=INT              Random seed (default: current Unix time)
 -q, --quiet                        Do not show progress messages
 -h, --help                         Show this message
```

 
### Learning strategy

`yskip` supports three learning strategies:
```
-l, --learning-strategy=INT        Learning strategy
                                    0: batch
                                    1: online
                                    2: mini-batch (default)
```

- The `batch` strategy first calculates the noise distribution from the entire training data, and then performs mini-batch SGD to learn skip-gram model.
- The `online` strategy performs the incremental SGNS described in [(Kaji and Kobayashi, 2017)](http://aclweb.org/anthology/D17-1037).
- The `mini-batch` updates the noise distribution every mini-batch, and performs mini-batch SGD to learn skip-gram model.


### Incremental learning

`yskip` is able to incrementally update an old model, which is specified by using `-I` option, to obtain a new one:
```
% cat text-1 | yskip - model.1
% cat text-2 | yskip -I model.1 - model.2
```
This produces the same result as the following command (if random seed is set the same).
```
% cat text-1 text-2 | yskip - model.2
```


## Converting the model file into word2vec-like format

The script `perl/to_word2vec.pl` converts yskip model file into word2vec-like format.
```   
perl/to_word2vec.pl < model.yskip > model.w2v
```


## Citation

Please cite [(Kaji and Kobayashi, 2017)](http://aclweb.org/anthology/D17-1037) when using this code to publish a paper.

```
@inproceedings{kaji17,
  author    = {Nobuhiro Kaji and Hayato Kobayashi},
  title     = {Incremental Skip-gram Model with Negative Sampling},
  booktitle = {Proceedings of EMNLP},
  year      = {2017},
  pages     = {363--371}
}
```

arXiv version is also available [here](https://arxiv.org/abs/1704.03956).
