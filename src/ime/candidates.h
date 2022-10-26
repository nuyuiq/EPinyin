#ifndef CANDIDATES_H
#define CANDIDATES_H

#include "dictdef.h"
#include <QList>
NAMESPACEBEGIN

struct LmaPsbItem
{
    // 32位对齐
    //size_t id: kLemmaIdSize * 8;
    quint32 id;
    quint16 lma_len;
    // The score, the lower psb, the higher possibility.
    quint16 psb;

    // 比较函数，后续针对候选排序
    bool operator <(const LmaPsbItem &other) const;
};

class Candidates
{
    QList<LmaPsbItem> list;
    typedef QList<LmaPsbItem>::ConstIterator ConstItr;
public:
    //! 候选词迭代器
    class Itr
    {
        ConstItr c;
        ConstItr e;
        inline Itr(ConstItr s, ConstItr e) : c(s), e(e) { }
    public:
        quint32 id() const { Q_ASSERT(c != e); return c->id; }
        bool next() { if (c != e) c++; return c != e; }
        friend class Candidates;
    };
    friend class Itr;

    //! 创建迭代器
    Itr pull(int offs, int len) const;

    //! 排序候选词
    void sortByPSB(int skip);

    //! 取某个候选词
    inline const LmaPsbItem &at(int idx) const;

    inline void reset();
    inline void append(const LmaPsbItem &item);
    inline int size() const;
    inline bool isFull() const;
};


const LmaPsbItem &Candidates::at(int idx) const
{
    Q_ASSERT(idx < size());
    return list.at(idx);
}

void Candidates::reset()
{
    list.clear();
}

void Candidates::append(const LmaPsbItem &item)
{
    Q_ASSERT(!isFull());
    list.append(item);
}

int Candidates::size() const
{
    return list.size();
}

bool Candidates::isFull() const
{
    return kMaxLmaPsbItems <= list.size();
}


NAMESPACEEND
#endif // CANDIDATES_H
