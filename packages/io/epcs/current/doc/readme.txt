Add CYGPKG_IO_EPCS package to ecos.ecc
To mount jffs2 over EPCS call mount function like this:  mount("/dev/epcs", "/", "jffs2")