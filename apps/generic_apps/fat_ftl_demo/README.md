This app demonstrates the usage of FAT filesystem library found in wiselib/wiselib.testing/utils/fat/fat.h

The app demonstrates basic functionality of FAT filesystem provided with The Wiselib. Please check wiselib/doc/concepts/file_system.dox for generic file system concept.

By default the app uses Flash memory along with FTL (FTL code provided in wiselib/wiselib.testing/algorithms/ftl/ftl.h), to use file blockmemory (easier to debug and check raw data) comment out lines using Flash memory and FTL, and uncomment lines using file block memory and pass the initialized block memory object to fs.init(..) and remove "Flash" parameter from typedef wiselib::Fat<Os,Flash> Fat;

The file blockmemory uses "myfile.img" as the instance of blockmemory.
