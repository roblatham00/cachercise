#include <vector>
#include <cstdint>
#include <cstddef>

#ifdef DEBUG_HOARD
#include <iostream>
#endif

/* just a big ol' flat array of data.  There is no paging out of excess
 * data.  no least recently used or anything like that.  Just how fast
 * can we update this data structure concurrently */

class Hoard {
    public:
        Hoard() = default;
        int put(int64_t * src, size_t count, size_t offset);
        int get(int64_t * dest, size_t count, size_t offset);
    private:
       std::vector<int64_t> m_hoard;
};

int Hoard::put(int64_t* src, size_t count, size_t offset)
{
    if (m_hoard.size() < offset + count)
        m_hoard.resize( (m_hoard.size()+offset+count) * 2);

    m_hoard.insert(m_hoard.begin()+offset, src, src+count);
#ifdef DEBUG_HOARD
    std::cout << "Hoard::put: " << src[0] << "...  " << count << " items at " << offset << "(hoard[0]: " << m_hoard[0] << std::endl;
#endif
    return 0;
}
int Hoard::get(int64_t *dest, size_t count, size_t offset)
{
#ifdef DEBUG_HOARD
    std::cout << "Hoard::get: " << count << " items at " << offset << " " << m_hoard[offset] << "(hoard[0]: "<<  m_hoard[0] << std::endl;
#endif
    for (size_t i= 0; i< count; i++)
        dest[i] = m_hoard[offset+i];
    return 0;
}
