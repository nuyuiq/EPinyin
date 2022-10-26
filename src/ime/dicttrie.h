#ifndef DICTTRIE_H
#define DICTTRIE_H

#include "ngram.h"
#include <QByteArray>
#include <QStringList>

NAMESPACEBEGIN

class Candidates;
class SpellingTrie;
class DictList;

/**
 * We use different node types for different layers
 * Statistical data of the building result for a testing dictionary:
 *                              root,   level 0,   level 1,   level 2,   level 3
 * max son num of one node:     406        280         41          2          -
 * max homo num of one node:      0         90         23          2          2
 * total node num of a layer:     1        406      31766      13516        993
 * total homo num of a layer:     9       5674      44609      12667        995
 *
 * The node number for root and level 0 won't be larger than 500
 * According to the information above, two kinds of nodes can be used; one for
 * root and level 0, the other for these layers deeper than 0.
 *
 * LE = less and equal,
 * A node occupies 16 bytes. so, totallly less than 16 * 500 = 8K
 */
struct LmaNodeLE0 {
    quint32 son_1st_off;
    quint32 homo_idx_buf_off;
    quint16 spl_idx;
    quint16 num_of_son;
    quint16 num_of_homo;
};

/**
 * GE = great and equal
 * A node occupies 8 bytes.
 */
struct LmaNodeGE1 {
    quint16 son_1st_off_l;        // Low bits of the son_1st_off
    quint16 homo_idx_buf_off_l;   // Low bits of the homo_idx_buf_off_1
    quint16 spl_idx;
    quint8 num_of_son;            // number of son nodes
    quint8 num_of_homo;           // number of homo words
    quint8 son_1st_off_h;         // high bits of the son_1st_off
    quint8 homo_idx_buf_off_h;    // high bits of the homo_idx_buf_off
};



class DictTrie
{
    QVector<LmaNodeLE0> root_;        // Nodes for root and the first layer.
    QVector<LmaNodeGE1> nodes_ge1_;   // Nodes for other layers.
    // The first part is for homophnies, and the last  top_lma_num_ items are
    // lemmas with highest scores.
    QByteArray lma_idx_buf_;
    // An quick index from spelling id to the LmaNodeLE0 node buffer, or
    // to the root_ buffer.
    // Index length:
    // SpellingTrie::get_instance().get_spelling_num() + 1. The last one is used
    // to get the end.
    // All Shengmu ids are not indexed because they will be converted into
    // corresponding full ids.
    // So, given an id splid, the son is:
    // root_[splid_le0_index_[splid - kFullSplIdStart]]
    QVector<quint16> splid_le0_index_;


    NGram *ngram;
    DictList *dictlist;

public:
    DictTrie();
    ~DictTrie();

    bool loadDictDict(QFile &fp, int spellingNum);
    bool loadDictList(QFile &fp);
    inline bool loadDictNGram(QFile &fp);

    int setCandidates(const quint16 *splidStr, int splidStrLen,
                             Candidates *candidates, const SpellingTrie *st) const;

    QStringList getCandidates(const Candidates *candidates, int offs, int len) const;

private:
    int getLpis(const quint16 *splidStr, int splidStrLen,
                    Candidates *candidates, const SpellingTrie *st) const;

    inline quint32 getLemmaId(size_t idOffset) const;
    inline size_t getSonOffset(const LmaNodeGE1 *node) const;
    inline size_t getHomoIdxBufOffset(const LmaNodeGE1 *node) const;
};

quint32 DictTrie::getLemmaId(size_t idOffset) const
{
    Q_ASSERT(kLemmaIdSize == 3);
    const char *p = lma_idx_buf_.constData() + idOffset * kLemmaIdSize;
    return
            (quint32 (quint8 (p[0])) << 0) +
            (quint32 (quint8 (p[1])) << 8) +
            (quint32 (quint8 (p[2])) << 16);
}

size_t DictTrie::getSonOffset(const LmaNodeGE1 *node) const
{
    return size_t (node->son_1st_off_l) +
            (size_t (node->son_1st_off_h) << 16);
}

size_t DictTrie::getHomoIdxBufOffset(const LmaNodeGE1 *node) const
{
    return size_t (node->homo_idx_buf_off_l) +
            (size_t (node->homo_idx_buf_off_h) << 16);
}



bool DictTrie::loadDictNGram(QFile &fp)
{
    return ngram->load(fp);
}

NAMESPACEEND

#endif // DICTTRIE_H
