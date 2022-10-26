#ifndef SPELLINGTRIE_H
#define SPELLINGTRIE_H

#include "dictdef.h"
#include <QtGlobal>
class QFile;

NAMESPACEBEGIN



// Node used for the trie of spellings
struct SpellingNode
{
    SpellingNode *first_son;
    // The spelling id for each node. If you need more bits to store
    // spelling id, please adjust this structure.
    quint16 spelling_idx:11;
    quint16  num_of_son:5;
    char char_this_node;
    quint8 score;
};

class  SpellingTrie
{
    // The spelling table
    char *spelling_buf_;

    // The size of longest spelling string, includes '\0' and an extra char to
    // store score. For example, "zhuang" is the longgest item in Pinyin list,
    // so spelling_size_ is 8.
    // Structure: The string ended with '\0' + score char.
    // An item with a lower score has a higher probability.
    quint32 spelling_size_;

    // Number of full spelling ids.
    quint32 spelling_num_;

    // The root node of the spelling tree
    SpellingNode root;

    // Used to get the first level sons.
    SpellingNode* level1_sons_[kValidSplCharNum];

    // The full spl_id range for specific half id.
    // h2f means half to full.
    // A half id can be a ShouZiMu id (id to represent the first char of a full
    // spelling, including Shengmu and Yunmu), or id of zh/ch/sh.
    // [1..kFullSplIdStart-1] is the arrange of half id.
    quint16 h2f_start_[kFullSplIdStart];
    quint16 h2f_num_[kFullSplIdStart];

    // Map from full id to half id.
    quint16 *f2h_;


    void freeSonTrie(SpellingNode* node);

    // Construct a subtree using a subset of the spelling array (from
    // item_star to item_end).
    // Member spelliing_buf_ and spelling_size_ should be valid.
    // parent is used to update its num_of_son and score.
    SpellingNode* constructSpellingsSubset(size_t itemStart, size_t itemEnd,
                                           size_t level, SpellingNode *parent);
    bool buildF2H();

    // Test if the given id is a valid spelling id.
    // If function returns true, the given splid may be updated like this:
    // When 'A' is not enabled in ShouZiMu mode, the parsing result for 'A' is
    // first given as a half id 1, but because 'A' is a one-char Yunmu and
    // it is a valid id, it needs to updated to its corresponding full id.
    bool ifValidIdUpdate(quint16 &splid) const;

public:
    SpellingTrie();
    ~SpellingTrie();

    inline static bool isValidSplChar(char ch);

    // The caller guarantees that the two chars are valid spelling chars.
    inline static bool isSameSplChar(char ch1, char ch2);

    // Test if the given id is a half id.
    inline static bool isHalfId(quint16 splid);

    // Test if the given id is a one-char Yunmu id (obviously, it is also a half
    // id), such as 'A', 'E' and 'O'.
    bool isHalfIdYunmu(quint16 splid) const;

    // Test If this char is enabled in ShouZiMu mode.
    // The caller should guarantee that ch >= 'A' && ch <= 'Z'
    bool szmIsEnabled(char ch) const;

    // Return the number of full ids for the given half id, and fill spl_id_start
    // to return the first full id.
    quint16 halfToFull(quint16 halfId, quint16 *splIdStart) const;

    // Load from the file stream
    bool loadSplTrie(QFile &fp);

    // Get the number of spellings
    inline quint32 getSpellingNum() const;


    // Given a string, parse it into a spelling id stream.
    // If the whole string are sucessfully parsed, last_is_pre will be true;
    // if the whole string is not fullly parsed, last_is_pre will return whether
    // the last part of the string is a prefix of a full spelling string. For
    // example, given string "zhengzhon", "zhon" is not a valid speling, but it is
    // the prefix of "zhong".
    //
    // If splstr starts with a character not in ['a'-z'] (it is a split char),
    // return 0.
    // Split char can only appear in the middle of the string or at the end.
    quint16 splstrToIdxs(const char *splstr, quint16 strLen, quint16 splIdx[],
                          quint16 startPos[], quint16 maxSize) const;

};

bool SpellingTrie::isValidSplChar(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool SpellingTrie::isSameSplChar(char ch1, char ch2)
{
    return ch1 == ch2 || ch1 - ch2 == 'a' - 'A' || ch2 - ch1 == 'a' - 'A';
}

bool SpellingTrie::isHalfId(quint16 splid)
{
    return 0 != splid && splid < kFullSplIdStart;
}

quint32 SpellingTrie::getSpellingNum() const
{
    return spelling_num_;
}


NAMESPACEEND

#endif // SPELLINGTRIE_H
