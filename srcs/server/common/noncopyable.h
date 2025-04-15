#ifndef INC_METIN_II_COMMON_NONCOPYABLE_TEMPLATE
#define INC_METIN_II_COMMON_NONCOPYABLE_TEMPLATE

class noncopyable
{
	protected:
		noncopyable() {}
		~noncopyable() {}

	private:
		noncopyable(const noncopyable &);
		noncopyable& operator = (const noncopyable &);
};

#endif
//martysama0134's b9b96f11f29d7eae7e2d2a9beb08ec8f
