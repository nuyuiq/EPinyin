#include "dictlist.h"
#include <QFile>

NAMESPACEBEGIN

bool DictList::load(QFile &fp)
{
    if (fp.read((char *)&scis_num_, 4) != 4) return false;
    if (fp.read((char *)&start_pos_, sizeof (start_pos_)) != sizeof (start_pos_)) return false;
    if (fp.read((char *)&start_id_, sizeof (start_id_)) != sizeof (start_pos_)) return false;
    buf_.resize(start_pos_[kMaxLemmaSize]);
    scis_hz_.resize(scis_num_);
    scis_splid_.resize(scis_num_);
    int size = scis_num_ * 2;
    if (fp.read((char *)scis_hz_.data(), size) != size) return false;
    if (fp.read((char *)scis_splid_.data(), size) != size) return false;
    size = start_pos_[kMaxLemmaSize] * 2;
    if (fp.read((char *)buf_.data(), size) != size) return false;
    return true;
}

QString DictList::getLemmaStr(quint32 id) const
{
    if (id < start_id_[kMaxLemmaSize])
    {
        // Find the range
        for (int i = 0; i < kMaxLemmaSize; i++)
        {
            if (start_id_[i] <= id && start_id_[i + 1] > id)
            {
                size_t idSpan = id - start_id_[i];
                const quint16 *buf = buf_.data() + start_pos_[i] + idSpan * (i + 1);
                return QString::fromUtf16(buf, i + 1);
            }
        }
    }
    return QString();
}

NAMESPACEEND


