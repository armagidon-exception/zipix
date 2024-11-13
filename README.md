# Zipix

## Description

Basic ZIP archiver written in C with support of Deflate and Shrink compression methods.
Point of this project is to learn compression algorithms of LZ\* family and make an application that is a little bit useful.

## Build

### Prequesites:

- gcc
- criterion
- make

```bash
make build
```

## Road map

- [x] Extract filesystem structure
- [x] Extract compressed file data
- [ ] Implementation of Deflate algorithm
- [ ] Implementation of Shrink algorithm
- [ ] Write to ZIP file
- [ ] Remove from ZIP file
- [ ] Basic GUI
- [ ] Adding stenography

## REFERENCES

- https://users.cs.jmu.edu/buchhofp/forensics/formats/pkzip.html
- https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
- https://www.hanshq.net/zip2.html#shrink
