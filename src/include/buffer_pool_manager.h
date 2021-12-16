#pragma once
#include "globals.h"
#include "page.h"
#include "lru_replacer.h"
#include "file_store.h"

/**
 * BufferPoolManager reads disk pages to and from its internal buffer pool.
 */
class BufferPoolManager {
public:
  /**
   * Creates a new BufferPoolManager.
   * @param poolSize the size of the buffer pool
   * @param fileStore the disk manager
   * @param enableCondVar indicates whether conditional variables are enabled
   */
  BufferPoolManager(size_t poolSize, FileStore *fileStore, bool enableCondVar = false);
  
  /**
   * Destroys an existing BufferPoolManager.
   */
  virtual ~BufferPoolManager();
  
  /** @return pointer to all the pages in the buffer pool */
  Page *getPages() { return m_pages; }
  
  /** @return size of the buffer pool */
  size_t getPoolSize() const { return m_poolSize; }
  
  /**
   * Fetch the requested page from the buffer pool.
   * @param fileType type of file which page belongs
   * @param pageID id of page to be fetched
   * @return the requested page
   */
  virtual Page *fetchPage(FileType fileType, PageIDType pageID);
  
  /**
   * Unpin the target page from the buffer pool.
   * @param fileType type of file which page belongs
   * @param pageID id of page to be unpinned
   * @param isDirty true if the page should be marked as dirty, false otherwise
   * @return false if the page pin count is <= 0 before this call, true otherwise
   */
  virtual bool unpinPage(FileType fileType, PageIDType pageID, bool isDirty);
  
  /**
   * Flushes the target page to disk.
   * @param fileType type of file which page belongs, cannot be FILE_TYPE::INVALID
   * @param pageID id of page to be flushed, cannot be INVALID_PAGE_ID
   * @return false if the page could not be found in the page table, true otherwise
   */
  bool flushPage(FileType fileType, PageIDType pageID);
  
  /**
   * Appends a new page to the file and fetch it.
   * @param fileType type of file appends new page to.
   * @param pageID id of new page to be appended.
   * @return nullptr if no new pages could be appended to file, otherwise pointer to new page
   */
  virtual Page *appendNewPage(FileType fileType, PageIDType pageID);
  
  /**
   * Flushes all the pages in the buffer pool to disk.
   */
  void flushAllPages();

private:
  Page *fetchExistentPage(FileType fileType, PageIDType pageID);

  Page *getVictimPage();

  bool flushPage_helper(FileType fileType, PageIDType pageID);

protected:
  /** Number of pages in the buffer pool. */
  size_t m_poolSize;
  /** Array of buffer pool pages. */
  Page *m_pages;
  /** Pointer to the file store. */
  FileStore *m_fileStore;
  /** Page table for keeping track of buffer pool pages. */
  std::map<std::pair<FileType, PageIDType>, FrameIDType> m_pageTable;
  /** Replacer to find unpinned pages for replacement. */
  Replacer *m_replacer;
  /** List of free pages. */
  std::list<FrameIDType> m_freeList;
  /** This latch protects shared data structures. */
  std::mutex m_poolLatch;
  /** This condition variable is used to ensure that the page is successfully fetched. */
  std::condition_variable m_poolCondition;
  /** Bool value indicating whether conditional variables are enabled **/
  bool m_enableCondVar;
};
