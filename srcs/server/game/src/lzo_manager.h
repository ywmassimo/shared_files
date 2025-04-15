#ifndef __INC_LZO_MANAGER_H
#define __INC_LZO_MANAGER_H

#include <lzo/lzo1x.h>

class LZOManager : public singleton<LZOManager>
{
	public:
		LZOManager();
		virtual ~LZOManager();

		bool	Compress(const BYTE* src, size_t srcsize, BYTE* dest, lzo_uint * puiDestSize);
		bool	Decompress(const BYTE* src, size_t srcsize, BYTE* dest, lzo_uint * puiDestSize);
		size_t	GetMaxCompressedSize(size_t original);

		BYTE *	GetWorkMemory() { return m_workmem; }

	private:
		BYTE *	m_workmem;
};

#endif
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
