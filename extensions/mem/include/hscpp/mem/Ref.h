#pragma once

#include "hscpp/mem/IMemoryManager.h"

namespace hscpp { namespace mem {

    template <typename T>
    class Ref
    {
        friend class MemoryManager;

    public:
        T* operator->() const
        {
            return GetMemory();
        }

        T* operator*() const
        {
            return GetMemory();
        }

        T* operator&() const
        {
            return GetMemory();
        }

    protected:
        uint64_t m_Id = IMemoryManager::INVALID_ID;
        IMemoryManager* m_pMemoryManager = nullptr;

        // Throws an std::runtime_error on a nullptr access. This makes it possible to catch
        // a null dereference on Linux and macOS with the hscpp::DoProtectedCall wrapper, and fix
        // the error during runtime (on Windows, a nullptr exception can always be caught).
        T* GetMemory() const
        {
            if (m_pMemoryManager == nullptr)
            {
                throw std::runtime_error("Ref is null (MemoryManager == nullptr).");
            }

            T* pT = reinterpret_cast<T*>(m_pMemoryManager->GetMemory(m_Id));
            if (pT == nullptr)
            {
                throw std::runtime_error("Ref is null.");
            }

            return pT;
        }

        // No std::runtime_error throw on a nullptr access.
        T* GetMemoryUnsafe() const
        {
            if (m_pMemoryManager == nullptr)
            {
                return nullptr;
            }

            return reinterpret_cast<T*>(m_pMemoryManager->GetMemory(m_Id));
        }

    };

    template <typename T>
    class UniqueRef : public Ref<T>
    {
        friend class MemoryManager;

        // Do not allow copy.
        UniqueRef(const UniqueRef<T>& rhs) = delete;
        UniqueRef<T>& operator=(const UniqueRef<T>& rhs) = delete;

    public:
        UniqueRef() = default;

        UniqueRef(UniqueRef<T>&& rhs) noexcept
        {
            MoveRef(std::move(rhs));
        }

        UniqueRef<T>& operator=(UniqueRef<T>&& rhs) noexcept
        {
            Free();
            MoveRef(std::move(rhs));
            return *this;
        }

        void Free()
        {
            if (this->m_Id == IMemoryManager::MEMORY_MANAGER_ID)
            {
                // This Ref refers to the MemoryManager itself. Do a normal delete, as the
                // MemoryManager will allocate no Block for its own instance.
                delete this->m_pMemoryManager;
                this->m_pMemoryManager = nullptr;
            }
            else
            {
                // Free memory, unless this Ref is a nullptr (double-freeing a Ref is safe).
                T* pSelf = this->GetMemoryUnsafe();
                if (pSelf != nullptr)
                {
                    pSelf->~T();
                    this->m_pMemoryManager->FreeBlock(this->m_Id, true);
                }
            }

            this->m_Id = IMemoryManager::INVALID_ID;
        }

        ~UniqueRef()
        {
            Free();
        }

    private:
        void MoveRef(UniqueRef<T>&& rhs) noexcept
        {
            this->m_Id = rhs.m_Id;
            this->m_pMemoryManager = rhs.m_pMemoryManager;

            rhs.m_Id = IMemoryManager::INVALID_ID;
            rhs.m_pMemoryManager = nullptr;
        }
    };

}}