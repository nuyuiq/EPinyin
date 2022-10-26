#include "epinyin.h"
#include "spellingtrie.h"
#include "dicttrie.h"
#include "candidates.h"
#include <QFile>

NAMESPACEBEGIN

EPinyin::EPinyin(const QString &dictfile)
{
    st = new SpellingTrie;
    dt = new DictTrie;
    cs = new Candidates;
    QFile f(dictfile);
    f.open(QIODevice::ReadOnly);
    bool b =
            st->loadSplTrie(f) &&
            dt->loadDictList(f) &&
            dt->loadDictDict(f, st->getSpellingNum()) &&
            dt->loadDictNGram(f);
    Q_ASSERT(b);
    Q_UNUSED(b);
    resetSearch();
}

EPinyin::~EPinyin()
{
    delete cs;
    delete dt;
    delete st;
}

size_t EPinyin::search(const QString &str)
{
    const QByteArray &py = str.toLatin1();
    return search(py.constData(), py.size());
}

size_t EPinyin::search(const char *py, int pyLen)
{
    pyLen = qMin(pyLen, kMaxRowNum - 1);

    // Compare the new string with the previous one. Find their prefix to
    // increase search efficiency.
    int chPos = 0;
    for (chPos = 0; chPos < pys_decoded_len_ && chPos < pyLen; chPos++)
    {
        if (py[chPos] != pys_[chPos]) break;
    }

    while (!fixed_spl_.isEmpty() && chPos < fixed_spl_.last())
    {
        cancelLastChoice0();
    }

    memcpy(pys_ + chPos, py + chPos, pyLen - chPos);
    pys_[pyLen] = '\0';
    pys_decoded_len_ = pyLen;

    return updateCandidate();
}

size_t EPinyin::choose(int idx)
{
    if (pys_decoded_len_ == 0 || idx >= cs->size())
    {
        return cs->size();
    }
    const LmaPsbItem &item = cs->at(idx);
    fixed_total_ += item.lma_len;
    fixed_list_.append(getCandidate(idx, 1));
    int splLst = spl_start_[item.lma_len];
    if (!fixed_spl_.isEmpty()) splLst += fixed_spl_.last();
    fixed_spl_.append(splLst);
    return updateCandidate();
}

size_t EPinyin::cancelLastChoice()
{
    if (fixed_list_.size())
    {
        cancelLastChoice0();
        return updateCandidate();
    }
    return cs->size();
}

QStringList EPinyin::getCandidate(int offs, int len) const
{
    return dt->getCandidates(cs, offs, len);
}

int EPinyin::getCandidateCount() const
{
    return cs->size();
}

QString EPinyin::getFixedStr() const
{
    return fixed_list_.join(QString());
}

void EPinyin::resetSearch()
{
    pys_decoded_len_ = 0;
    spl_id_num_ = 0;
    fixed_total_ = 0;
    fixed_list_.clear();
    fixed_spl_.clear();
    cs->reset();
}

void EPinyin::cancelLastChoice0()
{
    Q_ASSERT(fixed_total_ > 0);
    fixed_spl_.removeLast();
    fixed_total_ -= fixed_list_.takeLast().length();
    Q_ASSERT(fixed_total_ >= 0);
}

size_t EPinyin::updateCandidate()
{
    int pyOffs = fixed_spl_.isEmpty()? 0: fixed_spl_.last();
    int pyLen = pys_decoded_len_ - pyOffs;
    if (pyLen > 0)
    {
        spl_id_num_ = st->splstrToIdxs(
                    pys_ + pyOffs, pyLen,
                    spl_id_, spl_start_, kMaxRowNum - 1);

        dt->setCandidates(spl_id_, spl_id_num_, cs, st);
    }
    else
    {
        spl_id_num_ = 0;
        cs->reset();
    }
    return cs->size();
}

NAMESPACEEND
