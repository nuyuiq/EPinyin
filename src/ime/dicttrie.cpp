#include "dicttrie.h"
#include "dictlist.h"
#include "candidates.h"
#include "spellingtrie.h"
#include <QFile>

NAMESPACEBEGIN

DictTrie::DictTrie()
{
    dictlist = new DictList;
    ngram = new NGram;
}

DictTrie::~DictTrie()
{
    delete ngram;
    delete dictlist;
}

int DictTrie::getLpis(
        const quint16 *splidStr,
        int splidStrLen,
        Candidates *candidates,
        const SpellingTrie *st) const
{
    if (splidStrLen > kMaxLemmaSize)
        return 0;

#define MAX_EXTENDBUF_LEN 200
    const void* nodeBuf1[MAX_EXTENDBUF_LEN];  // use size_t for data alignment
    const void* nodeBuf2[MAX_EXTENDBUF_LEN];
    // Nodes from.
    const LmaNodeLE0** nodeFrLe0 =
            reinterpret_cast<const LmaNodeLE0**>(nodeBuf1);
    // Nodes to.
    const LmaNodeLE0** nodeToLe0 =
            reinterpret_cast<const LmaNodeLE0**>(nodeBuf2);
    const LmaNodeGE1** nodeFrGe1 = pNull;
    const LmaNodeGE1** nodeToGe1 = pNull;
    size_t nodeFrNum = 1;
    size_t nodeToNum = 0;
    const LmaNodeLE0 * const root = root_.data();
    const LmaNodeGE1 * const nodes_ge1 = nodes_ge1_.data();
    nodeFrLe0[0] = root;
    if (pNull == nodeFrLe0[0]) return 0;

    int splPos = 0;

    while (splPos < splidStrLen)
    {
        quint16 idNum = 1;
        quint16 idStart = splidStr[splPos];
        // If it is a half id
        if (SpellingTrie::isHalfId(splidStr[splPos]))
        {
            idNum = st->halfToFull(splidStr[splPos], &idStart);
            Q_ASSERT(idNum > 0);
        }

        // Extend the nodes
        if (0 == splPos) // From LmaNodeLE0 (root) to LmaNodeLE0 nodes
        {
            for (size_t nodeFrPos = 0; nodeFrPos < nodeFrNum; nodeFrPos++)
            {
                size_t sonStart = splid_le0_index_[idStart - kFullSplIdStart];
                size_t sonEnd = splid_le0_index_[idStart + idNum - kFullSplIdStart];
                for (size_t sonPos = sonStart; sonPos < sonEnd; sonPos++)
                {
                    const LmaNodeLE0 *nodeSon = root + sonPos;
                    if (nodeToNum < MAX_EXTENDBUF_LEN)
                    {
                        nodeToLe0[nodeToNum++] = nodeSon;
                    }
                    // id_start + id_num - 1 is the last one, which has just been
                    // recorded.
                    if (nodeSon->spl_idx >= idStart + idNum - 1)
                    {
                        break;
                    }
                }
            }
            splPos++;
            if (splPos >= splidStrLen || nodeToNum == 0)
            {
                break;
            }
            // Prepare the nodes for next extending
            // next time, from LmaNodeLE0 to LmaNodeGE1
            const LmaNodeLE0** nodeTmp = nodeFrLe0;
            nodeFrLe0 = nodeToLe0;
            nodeToLe0 = pNull;
            nodeToGe1 = reinterpret_cast<const LmaNodeGE1**>(nodeTmp);
        }
        else if (1 == splPos) // From LmaNodeLE0 to LmaNodeGE1 nodes
        {
            for (size_t nodeFrPos = 0; nodeFrPos < nodeFrNum; nodeFrPos++)
            {
                const LmaNodeLE0 *node = nodeFrLe0[nodeFrPos];
                for (size_t sonPos = 0; sonPos < size_t(node->num_of_son); sonPos++)
                {
                    const LmaNodeGE1 *nodeSon = nodes_ge1_.data() + node->son_1st_off + sonPos;
                    if (nodeSon->spl_idx >= idStart && nodeSon->spl_idx < idStart + idNum)
                    {
                        if (nodeToNum < MAX_EXTENDBUF_LEN)
                        {
                            nodeToGe1[nodeToNum++] = nodeSon;
                        }
                    }
                    // id_start + id_num - 1 is the last one, which has just been
                    // recorded.
                    if (nodeSon->spl_idx >= idStart + idNum - 1)
                    {
                        break;
                    }
                }
            }
            splPos++;
            if (splPos >= splidStrLen || nodeToNum == 0)
            {
                break;
            }
            // Prepare the nodes for next extending
            // next time, from LmaNodeGE1 to LmaNodeGE1
            nodeFrGe1 = nodeToGe1;
            nodeToGe1 = reinterpret_cast<const LmaNodeGE1**>(nodeFrLe0);
            nodeFrLe0 = pNull;
            nodeToLe0 = pNull;
        }
        else // From LmaNodeGE1 to LmaNodeGE1 nodes
        {
            for (size_t nodeFrPos = 0; nodeFrPos < nodeFrNum; nodeFrPos++)
            {
                const LmaNodeGE1 *node = nodeFrGe1[nodeFrPos];
                for (size_t sonPos = 0; sonPos < size_t (node->num_of_son); sonPos++)
                {
                    const LmaNodeGE1 *nodeSon = nodes_ge1 + getSonOffset(node) + sonPos;
                    if (nodeSon->spl_idx >= idStart && nodeSon->spl_idx < idStart + idNum)
                    {
                        if (nodeToNum < MAX_EXTENDBUF_LEN)
                        {
                            nodeToGe1[nodeToNum++] = nodeSon;
                        }
                    }
                    // id_start + id_num - 1 is the last one, which has just been
                    // recorded.
                    if (nodeSon->spl_idx >= idStart + idNum - 1)
                    {
                        break;
                    }
                }
            }
            splPos++;
            if (splPos >= splidStrLen || nodeToNum == 0)
            {
                break;
            }
            // Prepare the nodes for next extending
            // next time, from LmaNodeGE1 to LmaNodeGE1
            const LmaNodeGE1 **nodeTmp = nodeFrGe1;
            nodeFrGe1 = nodeToGe1;
            nodeToGe1 = nodeTmp;
        }

        // The number of node for next extending
        nodeFrNum = nodeToNum;
        nodeToNum = 0;
    }

    if (0 == nodeToNum) return 0;

    // If the length is 1, and the splid is a one-char Yunmu like 'a', 'o', 'e',
    // only those candidates for the full matched one-char id will be returned.
    if (1 == splidStrLen && st->isHalfIdYunmu(splidStr[0]))
    {
        nodeToNum = nodeToNum > 0 ? 1 : 0;
    }
    LmaPsbItem item;
    for (size_t nodePos = 0; nodePos < nodeToNum; nodePos++)
    {
        size_t numOfHomo = 0;
        if (splPos <= 1) // Get from LmaNodeLE0 nodes
        {
            const LmaNodeLE0* nodeLe0 = nodeToLe0[nodePos];
            numOfHomo = nodeLe0->num_of_homo;
            for (size_t homoPos = 0; homoPos < numOfHomo; homoPos++)
            {
                item.id = getLemmaId(nodeLe0->homo_idx_buf_off + homoPos);
                item.lma_len = 1;
                item.psb = ngram->getUniPSB(item.id);
                candidates->append(item);
                if (candidates->isFull()) break;
            }
        }
        else // Get from LmaNodeGE1 nodes
        {
            const LmaNodeGE1* nodeGe1 = nodeToGe1[nodePos];
            numOfHomo = nodeGe1->num_of_homo;
            for (size_t homoPos = 0; homoPos < numOfHomo; homoPos++)
            {
                size_t nodeHomoOff = getHomoIdxBufOffset(nodeGe1);
                item.id = getLemmaId(nodeHomoOff + homoPos);
                item.lma_len = quint16 (splidStrLen);
                item.psb = ngram->getUniPSB(item.id);
                candidates->append(item);
                if (candidates->isFull()) break;
            }
        }
        if (candidates->isFull()) break;
    }
    return candidates->size();
}

bool DictTrie::loadDictDict(QFile &fp, int spellingNum)
{
    int lma_node_num_le0_;
    int lma_node_num_ge1_;
    int lma_idx_buf_len_;  // The total size of lma_idx_buf_ in byte.
    int top_lmas_num_;     // Number of lemma with highest scores.
    if (fp.read((char *)&lma_node_num_le0_, 4) != 4) return false;
    if (fp.read((char *)&lma_node_num_ge1_, 4) != 4) return false;
    if (fp.read((char *)&lma_idx_buf_len_, 4) != 4) return false;
    if (fp.read((char *)&top_lmas_num_, 4) != 4) return false;

    root_.resize(lma_node_num_le0_);
    nodes_ge1_.resize(lma_node_num_ge1_);
    lma_idx_buf_.resize(lma_idx_buf_len_);
    int buf_size = spellingNum + 1;
    splid_le0_index_.resize(buf_size);

    //    parsing_marks_ = new ParsingMark[kMaxParsingMark];
    //    mile_stones_ = new MileStone[kMaxMileStone];

    int size = sizeof(LmaNodeLE0) * root_.size();
    if (fp.read((char *)root_.data(), size) != size) return false;
    size = sizeof(LmaNodeGE1) * nodes_ge1_.size();
    if (fp.read((char *)nodes_ge1_.data(), size) != size) return false;
    if (fp.read(lma_idx_buf_.data(), lma_idx_buf_.size()) != lma_idx_buf_.size()) return false;

    // The quick index for the first level sons
    quint16 last_splid = kFullSplIdStart;
    int last_pos = 0;
    for (int i = 1; i < lma_node_num_le0_; i++)
    {
        for (quint16 splid = last_splid; splid < root_[i].spl_idx; splid++)
        {
            splid_le0_index_[splid - kFullSplIdStart] = quint16 (last_pos);
        }
        splid_le0_index_[root_.at(i).spl_idx - kFullSplIdStart] = quint16 (i);
        last_splid = root_.at(i).spl_idx;
        last_pos = i;
    }

    for (quint16 splid = last_splid + 1;
         splid < buf_size + kFullSplIdStart; splid++)
    {
        Q_ASSERT(splid - kFullSplIdStart < buf_size);
        splid_le0_index_[splid - kFullSplIdStart] = quint16 (last_pos + 1);
    }

    return true;
}

bool DictTrie::loadDictList(QFile &fp)
{
    return dictlist->load(fp);
}

int DictTrie::setCandidates(const quint16 *splidStr,
        int splidStrLen,
        Candidates *candidates,
        const SpellingTrie *st) const
{
    // Get candiates from the first un-fixed step.
    int lmaSize = qMin(kMaxLemmaSize, splidStrLen);
    // Number of items which are fully-matched.
    int lpi_num_full_match = 0;
    candidates->reset();
    while (lmaSize > 0)
    {
        getLpis(splidStr, lmaSize, candidates, st);
        if (lmaSize == splidStrLen)
        {
            candidates->sortByPSB(0);
            lpi_num_full_match = candidates->size();
        }
        lmaSize--;
    }
    candidates->sortByPSB(lpi_num_full_match);
    return candidates->size();
}

QStringList DictTrie::getCandidates(const Candidates *candidates, int offs, int len) const
{
    IME::Candidates::Itr itr = candidates->pull(offs, len);
    QStringList ls;
    while (itr.next())
    {
        ls << dictlist->getLemmaStr(itr.id());
    }
    return ls;
}


NAMESPACEEND
