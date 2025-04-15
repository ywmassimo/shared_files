#include "desc.h"

class DESC_P2P : public DESC
{
	public:
		virtual ~DESC_P2P();

		virtual void	Destroy();
		virtual void	SetPhase(int iPhase);
		bool		Setup(LPFDWATCH _fdw, socket_t fd, const char * host, WORD wPort);
};
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
