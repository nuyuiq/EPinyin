#ifndef EPINYIN_H
#define EPINYIN_H

#include "dictdef.h"
#include <QVector>
#include <QStringList>

NAMESPACEBEGIN

class SpellingTrie;
class DictTrie;
class Candidates;

class EPinyin
{
    void cancelLastChoice0();
    size_t updateCandidate();
public:
    EPinyin(const QString &dictfile);
    ~EPinyin();

    // Search a Pinyin string.
    // Return value is the position successfully parsed.
    size_t search(const QString &str);
    size_t search(const char *py, int pyLen);
    // Choose a candidate. The decoder will do a search after the fixed position.
    size_t choose(int idx);
    size_t cancelLastChoice();
    QStringList getCandidate(int offs, int len) const;
    QString getFixedStr() const;
    void resetSearch();

    int getCandidateCount() const;
    inline int getFixedSplLen() const;
    inline const char* getSpsStr(int *len) const;
    inline const quint16 *getSplStartPos(int *len) const;

private:
    SpellingTrie *st;
    DictTrie *dt;
    Candidates *cs;

    // Used to remember the last fixed position, counted in Hanzi.
    QList<int> fixed_spl_;
    QStringList fixed_list_;
    int fixed_total_;

    // The length of the string that has been decoded successfully.
    int pys_decoded_len_;

    // Number of splling ids
    int spl_id_num_;

    // Starting positions
    quint16 spl_start_[kMaxRowNum];

    // Spelling ids
    quint16 spl_id_[kMaxRowNum];

    // Pinyin string. Max length: kMaxRowNum - 1
    char pys_[kMaxRowNum];
};



int EPinyin::getFixedSplLen() const
{
    return fixed_spl_.isEmpty()? 0: fixed_spl_.last();
}

const char *EPinyin::getSpsStr(int *len) const
{
    *len = pys_decoded_len_;
    return pys_;
}

const quint16 * EPinyin::getSplStartPos(int *len) const
{
    *len = spl_id_num_;
    return spl_start_;
}

NAMESPACEEND

#endif // EPINYIN_H
