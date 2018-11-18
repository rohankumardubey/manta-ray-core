#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <assert.h>
#include <new>

namespace manta {

	class StackAllocator {
	public:
		StackAllocator();
		~StackAllocator();

		void initialize(unsigned int size);

		void *allocate(unsigned int size, unsigned int alignment = 1) {
#ifdef _DEBUG
			m_allocationLedger++;
#endif
			void *realAddress = m_stackPointer;

			// Find an aligned address
			unsigned __int64 stackPtr = (unsigned __int64)m_stackPointer;
			unsigned int mod = (alignment - stackPtr % alignment) % alignment;

			void *basePointer = (void *)((char *)m_stackPointer + mod);

			void *newObject = basePointer;

			// Update previous block pointers
			m_stackPointer = (void *)((char *)basePointer + size);

			return newObject;
		}

		void free(void *memory) {
#ifdef _DEBUG
			m_allocationLedger--;
			assert(m_allocationLedger >= 0);
#endif
			assert((unsigned __int64)memory < (unsigned __int64)m_stackPointer);

			// Return the stack pointer to the beginning of the block
			m_stackPointer = memory;
		}

	protected:
		void *m_buffer;
		void *m_stackPointer;

		unsigned int m_size;

		// Statistics counters
		int m_allocationLedger;
	};

}

#endif /* STACK_ALLOCATOR_H */