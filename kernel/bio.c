// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#define BUFMAP_HASH(dev,blockno) ((((dev)<<27)|(blockno))%NBUFMAP_BUCKET)

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock eviction_lock;
  struct buf buf[NBUF];


  struct buf bufmap[NBUFMAP_BUCKET];
  struct spinlock bufmap_lock[NBUFMAP_BUCKET];
} bcache;

void
binit(void)
{
  

  initlock(&bcache.eviction_lock,"bcache_eviction");
  for(int i=0;i<NBUFMAP_BUCKET;i++){
    initlock(&bcache.bufmap_lock[i],"bcache_bufmap");
    bcache.bufmap[i].next=0;
  }

  
  for(int i=0; i<NBUF; i++){
    struct buf *b=&bcache.buf[i];
    b->next=0;
    initsleeplock(&b->lock,"buffer");
    b->lastuse=0;
    b->refcnt=0;
    b->next=bcache.bufmap[0].next;
    bcache.bufmap[0].next=b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  uint key=BUFMAP_HASH(dev,blockno);

  acquire(&bcache.bufmap_lock[key]);

  // Is the block already cached?
  for(b = bcache.bufmap[key].next; b != 0; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bufmap_lock[key]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  

  // Not cached.
  release(&bcache.bufmap_lock[key]);
  acquire(&bcache.eviction_lock);

  for(b = bcache.bufmap[key].next; b != 0; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      acquire(&bcache.bufmap_lock[key]);
      b->refcnt++;
      release(&bcache.bufmap_lock[key]);
      release(&bcache.eviction_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  struct buf*least_used=0;
  uint holding_bucket=-1;
  for(int i=0;i<NBUFMAP_BUCKET;i++){
    acquire(&bcache.bufmap_lock[i]);
    int newfound=0;

    for(b = &bcache.bufmap[i]; b->next != 0; b = b->next){
      if(b->next->refcnt == 0&&(!least_used||b->next->lastuse<least_used->next->lastuse)) {
       least_used=b;
        newfound=1;
      }
    }

    if(!newfound){
      release(&bcache.bufmap_lock[i]);
    }else{
      if(holding_bucket!=-1)release(&bcache.bufmap_lock[holding_bucket]);
      holding_bucket=i;
    }
  }
  if(least_used==0)panic("bget: no buffers");

  b=least_used->next;  

  if(holding_bucket!=key){
    least_used->next=least_used->next->next;
    release(&bcache.bufmap_lock[holding_bucket]);
    acquire(&bcache.bufmap_lock[key]);
    b->next=bcache.bufmap[key].next;
    bcache.bufmap[key].next=b;
  }

    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    release(&bcache.bufmap_lock[key]);
    release(&bcache.eviction_lock);
    acquiresleep(&b->lock);
    return b;
  
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  uint key=BUFMAP_HASH(b->dev,b->blockno);

  acquire(&bcache.bufmap_lock[key]);
  b->refcnt--;
  if (b->refcnt == 0) {
    b->lastuse=ticks;
  }
  
  release(&bcache.bufmap_lock[key]);
}

void
bpin(struct buf *b) {
  uint key=BUFMAP_HASH(b->dev,b->blockno);
  acquire(&bcache.bufmap_lock[key]);
  b->refcnt++;
  release(&bcache.bufmap_lock[key]);
}

void
bunpin(struct buf *b) {
  uint key=BUFMAP_HASH(b->dev,b->blockno);
  acquire(&bcache.bufmap_lock[key]);
  b->refcnt--;
  release(&bcache.bufmap_lock[key]);
}


