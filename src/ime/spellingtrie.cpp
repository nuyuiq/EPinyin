#include "spellingtrie.h"
#include <QFile>

NAMESPACEBEGIN

// Map from half spelling id to single char.
// For half ids of Zh/Ch/Sh, map to z/c/s (low case) respectively.
// For example, 1 to 'A', 2 to 'B', 3 to 'C', 4 to 'c', 5 to 'D', ...,
// 28 to 'Z', 29 to 'z'.
// [0] is not used to achieve better efficiency.
// z/c/s is for Zh/Ch/Sh
static const char kHalfId2Sc_[kFullSplIdStart + 1] =
    "0ABCcDEFGHIJKLMNOPQRSsTUVWXYZz";

// Bit 0 : is it a Shengmu char?
// Bit 1 : is it a Yunmu char? (one char is a Yunmu)
// Bit 2 : is it enabled in ShouZiMu(first char) mode?
static const unsigned char char_flags_[] = {
  // a    b      c     d     e     f     g
  0x06, 0x05, 0x05, 0x05, 0x06, 0x05, 0x05,
  // h    i     j      k     l     m    n
  0x05, 0x00, 0x05, 0x05, 0x05, 0x05, 0x05,
  // o    p     q      r     s     t
  0x06, 0x05, 0x05, 0x05, 0x05, 0x05,
  // u    v     w      x     y     z
  0x00, 0x00, 0x05, 0x05, 0x05, 0x05
};

#define kHalfIdShengmuMask 0x01
#define kHalfIdYunmuMask   0x02
#define kHalfIdSzmMask     0x04

// The caller should guarantee ch >= 'A' && ch <= 'Z'
static inline bool isShengmuChar(char ch)
{
    return char_flags_[ch - 'A'] & kHalfIdShengmuMask;
}

// The caller should guarantee ch >= 'A' && ch <= 'Z'
static inline bool isYunmuChar(char ch)
{
    return char_flags_[ch - 'A'] & kHalfIdYunmuMask;
}

// Test if this char is a ShouZiMu char. This ShouZiMu char may be not enabled.
// For Pinyin, only i/u/v is not a ShouZiMu char.
// The caller should guarantee that ch >= 'A' && ch <= 'Z'
static inline bool isSzmChar(char ch)
{
    return isShengmuChar(ch) || isYunmuChar(ch);
}

void SpellingTrie::freeSonTrie(SpellingNode *node)
{
    if (pNull == node) return;
    for (size_t pos = 0; pos < node->num_of_son; pos++)
    {
        freeSonTrie(node->first_son + pos);
    }
    if (pNull != node->first_son) delete [] node->first_son;
}

SpellingNode *SpellingTrie::constructSpellingsSubset(
        size_t itemStart,
        size_t itemEnd,
        size_t level,
        SpellingNode *parent)
{
    if (level >= spelling_size_ || itemEnd <= itemStart || pNull == parent)
    {
        return pNull;
    }
    SpellingNode *firstSon = pNull;
    quint16 numOfSon = 0;
    quint8 minSonScore = 255;

    const char *spellingLastStart = spelling_buf_ + spelling_size_ * itemStart;
    char charForNode = spellingLastStart[level];
    Q_ASSERT((charForNode >= 'A' && charForNode <= 'Z') || 'h' == charForNode);

    // Scan the array to find how many sons
    for (size_t i = itemStart + 1; i < itemEnd; i++)
    {
        const char *spellingCurrent = spelling_buf_ + spelling_size_ * i;
        char charCurrent = spellingCurrent[level];
        if (charCurrent != charForNode)
        {
            numOfSon++;
            charForNode = charCurrent;
        }
    }
    numOfSon++;

    // Allocate memory
    firstSon = new SpellingNode[numOfSon];
    memset(firstSon, 0, sizeof(SpellingNode) * numOfSon);

    // Now begin construct tree
    size_t sonPos = 0;

    spellingLastStart = spelling_buf_ + spelling_size_ * itemStart;
    charForNode = spellingLastStart[level];

    bool spellingEndable = true;
    if (spellingLastStart[level + 1] != '\0')
    {
        spellingEndable = false;
    }
    size_t itemStartNext = itemStart;

    const char *spellingCurrent = pNull;
    char charCurrent = 0;
    for (size_t i = itemStart + 1; i <= itemEnd; i++)
    {
        if (i != itemEnd)
        {
            spellingCurrent = spelling_buf_ + spelling_size_ * i;
            charCurrent = spellingCurrent[level];
            Q_ASSERT(isValidSplChar(charCurrent));
            if (charCurrent == charForNode) continue;
        }

        // Construct a node
        SpellingNode *nodeCurrent = firstSon + sonPos;
        nodeCurrent->char_this_node = charForNode;

        // For quick search in the first level
        if (0 == level)
        {
            level1_sons_[charForNode - 'A'] = nodeCurrent;
        }
        if (spellingEndable)
        {
            nodeCurrent->spelling_idx = kFullSplIdStart + itemStartNext;
        }

        if (spellingLastStart[level + 1] != '\0' || i - itemStartNext > 1)
        {
            size_t realStart = itemStartNext;
            if (spellingLastStart[level + 1] == '\0')
            {
                realStart++;
            }
            nodeCurrent->first_son =
                    constructSpellingsSubset(realStart, i, level + 1, nodeCurrent);

            if (realStart == itemStartNext + 1)
            {
                quint16 scoreThis = quint8(spellingLastStart[spelling_size_ - 1]);
                if (scoreThis < nodeCurrent->score)
                {
                    nodeCurrent->score = scoreThis;
                }
            }
        }
        else
        {
            nodeCurrent->first_son = pNull;
            nodeCurrent->score = quint8(spellingLastStart[spelling_size_ - 1]);
        }

        if (nodeCurrent->score < minSonScore)
        {
            minSonScore = nodeCurrent->score;
        }
        bool isHalf = false;
        if (level == 0 && (i != itemEnd? isSzmChar(charForNode): szmIsEnabled(charForNode)))
        {
            nodeCurrent->spelling_idx = quint16(charForNode - 'A' + 1);

            if (charForNode > 'C') nodeCurrent->spelling_idx++;
            if (charForNode > 'S') nodeCurrent->spelling_idx++;

            h2f_num_[nodeCurrent->spelling_idx] = i - itemStartNext;
            isHalf = true;
        }
        else if (level == 1 && charForNode == 'h')
        {
            char chLevel0 = spellingLastStart[0];
            quint16 partId = 0;
            if (chLevel0 == 'C') partId = 'C' - 'A' + 1 + 1;
            else if (chLevel0 == 'S') partId = 'S' - 'A' + 1 + 2;
            else if (chLevel0 == 'Z') partId = 'Z' - 'A' + 1 + 3;
            if (0 != partId)
            {
                nodeCurrent->spelling_idx = partId;
                h2f_num_[nodeCurrent->spelling_idx] = i - itemStartNext;
                isHalf = true;
            }
        }

        if (isHalf)
        {
            if (h2f_num_[nodeCurrent->spelling_idx] > 0)
            {
                h2f_start_[nodeCurrent->spelling_idx] =
                        itemStartNext + kFullSplIdStart;
            }
            else
            {
                h2f_start_[nodeCurrent->spelling_idx] = 0;
            }
        }

        if (i != itemEnd)
        {
            // for next sibling
            spellingLastStart = spellingCurrent;
            charForNode = charCurrent;
            itemStartNext = i;
            spellingEndable = true;
            if (spellingCurrent[level + 1] != '\0')
            {
                spellingEndable = false;
            }
            sonPos++;
        }
    }

    parent->num_of_son = numOfSon;
    parent->score = minSonScore;
    return firstSon;
}

bool SpellingTrie::buildF2H()
{
    f2h_ = new quint16[spelling_num_];
    Q_ASSERT(f2h_);

    for (quint16 hid = 0; hid < kFullSplIdStart; hid++)
    {
        int h2fEnd = h2f_start_[hid] + h2f_num_[hid];
        for (quint16 fid = h2f_start_[hid]; fid < h2fEnd; fid++)
        {
            f2h_[fid - kFullSplIdStart] = hid;
        }
    }
    return true;
}

SpellingTrie::SpellingTrie()
{
    spelling_buf_ = pNull;
    spelling_size_ = 0;
    spelling_num_ = 0;
    f2h_ = pNull;
    memset(&root, 0, sizeof(SpellingNode));
}

SpellingTrie::~SpellingTrie()
{
    freeSonTrie(&root);
    if (f2h_)          delete [] f2h_;
    if (spelling_buf_) delete [] spelling_buf_;
}

bool SpellingTrie::ifValidIdUpdate(quint16 &splid) const
{
    if (0 == splid) return false;

    if (splid >= kFullSplIdStart) return true;
    if (splid < kFullSplIdStart)
    {
        char ch = kHalfId2Sc_[splid];
        if (ch > 'Z') return true;
        if (szmIsEnabled(ch)) return true;
        if (isYunmuChar(ch))
        {
            Q_ASSERT(h2f_num_[splid] > 0);
            splid = h2f_start_[splid];
            return true;
        }
    }
    return false;
}

bool SpellingTrie::isHalfIdYunmu(quint16 splid) const
{
    if (0 == splid || splid >= kFullSplIdStart) return false;

    char ch = kHalfId2Sc_[splid];
    // If ch >= 'a', that means the half id is one of Zh/Ch/Sh
    if (ch >= 'a')
    {
        return false;
    }
    return char_flags_[ch - 'A'] & kHalfIdYunmuMask;
}

bool SpellingTrie::szmIsEnabled(char ch) const
{
    return char_flags_[ch - 'A'] & kHalfIdSzmMask;
}

quint16 SpellingTrie::halfToFull(quint16 halfId, quint16 *splIdStart) const
{
    if (pNull == splIdStart || halfId >= kFullSplIdStart)
    {
        return 0;
    }
    *splIdStart = h2f_start_[halfId];
    return h2f_num_[halfId];
}

bool SpellingTrie::loadSplTrie(QFile &fp)
{
    if (fp.read((char *)&spelling_size_, 4) != 4) return false;
    if (fp.read((char *)&spelling_num_, 4) != 4) return false;

    float scoreAmplifier;
    unsigned char averageScore;
    if (fp.read((char *)&scoreAmplifier, sizeof(float)) != sizeof(float)) return false;
    if (fp.read((char *)&averageScore, 1) != 1) return false;

    spelling_buf_ = new char[spelling_size_ * spelling_num_];
    if (pNull == spelling_buf_) return false;

    const int size = spelling_size_ * spelling_num_;
    if (fp.read((char *)spelling_buf_, size) != size) return false;

    memset(&root, 0, sizeof(SpellingNode));
    memset(level1_sons_, 0, sizeof(SpellingNode*) * kValidSplCharNum);

    memset(h2f_start_, 0, sizeof(quint16) * kFullSplIdStart);
    memset(h2f_num_, 0, sizeof(quint16) * kFullSplIdStart);

    root.first_son = constructSpellingsSubset(0, spelling_num_, 0, &root);
    if (pNull == root.first_son) return false;

    return buildF2H();
}

quint16 SpellingTrie::splstrToIdxs(
        const char *splstr,
        quint16 strLen,
        quint16 splIdx[],
        quint16 startPos[],
        quint16 maxSize) const
{
    if (pNull == splstr || 0 == maxSize || 0 == strLen) return 0;
    if (!SpellingTrie::isValidSplChar(splstr[0])) return 0;

    const SpellingNode *nodeThis = &root;

    quint16 strPos = 0;
    quint16 idxNum = 0;
    if (pNull != startPos) startPos[0] = 0;
    bool lastIsSplitter = false;

    while (strPos < strLen)
    {
        char charThis = splstr[strPos];
        // all characters outside of [a, z] are considered as splitters
        if (!SpellingTrie::isValidSplChar(charThis))
        {
            // test if the current node is endable
            quint16 idThis = nodeThis->spelling_idx;
            if (ifValidIdUpdate(idThis))
            {
                splIdx[idxNum] = idThis;
                idxNum++;
                strPos++;
                if (pNull != startPos) startPos[idxNum] = strPos;
                if (idxNum >= maxSize) return idxNum;

                nodeThis = &root;
                lastIsSplitter = true;
                continue;
            }
            else  if (lastIsSplitter)
            {
                strPos++;
                if (pNull != startPos) startPos[idxNum] = strPos;
                continue;
            }
            else
            {
                return idxNum;
            }
        }

        lastIsSplitter = false;
        SpellingNode *foundSon = pNull;
        if (0 == strPos)
        {
            if (charThis >= 'a')
            {
                foundSon = level1_sons_[charThis - 'a'];
            }
            else
            {
                foundSon = level1_sons_[charThis - 'A'];
            }
        }
        else
        {
            SpellingNode *firstSon = nodeThis->first_son;
            // Because for Zh/Ch/Sh nodes, they are the last in the buffer and
            // frequently used, so we scan from the end.
            for (int i = 0; i < nodeThis->num_of_son; i++)
            {
                SpellingNode *thisSon = firstSon + i;
                if (SpellingTrie::isSameSplChar(thisSon->char_this_node, charThis))
                {
                    foundSon = thisSon;
                    break;
                }
            }
        }

        // found, just move the current node pointer to the the son
        if (pNull != foundSon)
        {
            nodeThis = foundSon;
        }
        else
        {
            // not found, test if it is endable
            quint16 idThis = nodeThis->spelling_idx;
            if (ifValidIdUpdate(idThis))
            {
                // endable, remember the index
                splIdx[idxNum] = idThis;

                idxNum++;
                if (pNull != startPos) startPos[idxNum] = strPos;
                if (idxNum >= maxSize) return idxNum;
                nodeThis = &root;
                continue;
            }
            else
            {
                return idxNum;
            }
        }
        strPos++;
    }

    quint16 idThis = nodeThis->spelling_idx;
    if (ifValidIdUpdate(idThis))
    {
        // endable, remember the index
        splIdx[idxNum] = idThis;
        idxNum++;
        if (pNull != startPos) startPos[idxNum] = strPos;
    }
    return idxNum;
}


NAMESPACEEND
