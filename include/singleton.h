#ifndef CUB_SINGLETON_H
#define CUB_SINGLETON_H

#include <assert.h>

template <class T>
class Singleton
{
    public:
        static T* instance()
        {
            if (!m_instance)
                m_instance = new T;

            assert(m_instance != NULL);

            return m_instance;
        }

    private:
        static T* m_instance;
};

template <class T> T* Singleton<T>::m_instance = NULL;

#endif
