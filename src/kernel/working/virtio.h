// virtio mmio control registers offset from base address
// from qemu virtio_mmio.h
#define VIRTIO0                         0x10001000  // virtio base address
#define VIRTIO_MMIO_MAGIC_VALUE		    0x00 // 0x74726976
#define VIRTIO_MMIO_VERSION		        0x04 // version; should be 2
#define VIRTIO_MMIO_DEVICE_ID		    0x08 // device type; 1 is net, 2 is disk
#define VIRTIO_MMIO_VENDOR_ID		    0x0c // 0x554d4551
#define VIRTIO_MMIO_DEVICE_FEATURES	    0x10
#define VIRTIO_MMIO_DRIVER_FEATURES 	0x20
#define VIRTIO_MMIO_QUEUE_SEL		    0x30 // select queue, write-only
#define VIRTIO_MMIO_QUEUE_NUM_MAX	    0x34 // max size of current queue, read-only
#define VIRTIO_MMIO_QUEUE_NUM		    0x38 // size of current queue, write-only
#define VIRTIO_MMIO_QUEUE_READY		    0x44 // ready bit
#define VIRTIO_MMIO_QUEUE_NOTIFY	    0x50 // write-only
#define VIRTIO_MMIO_INTERRUPT_STATUS	0x60 // read-only
#define VIRTIO_MMIO_INTERRUPT_ACK	    0x64 // write-only
#define VIRTIO_MMIO_STATUS		        0x70 // read/write
#define VIRTIO_MMIO_QUEUE_DESC_LOW	    0x80 // physical address for descriptor table, write-only
#define VIRTIO_MMIO_QUEUE_DESC_HIGH	    0x84
#define VIRTIO_MMIO_DRIVER_DESC_LOW	    0x90 // physical address for available ring, write-only
#define VIRTIO_MMIO_DRIVER_DESC_HIGH	0x94
#define VIRTIO_MMIO_DEVICE_DESC_LOW	    0xa0 // physical address for used ring, write-only
#define VIRTIO_MMIO_DEVICE_DESC_HIGH	0xa4

// macro fucntion to get the address of virtio registers. Base address is defined in phymem.h
#define virtio_read_reg(reg) ((volatile uint32 *)(VIRTIO0 + (reg)))

void disk_init(void);