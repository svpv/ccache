typedef FILE *gzFile;
#define gzopen fopen
#define gzdopen fdopen
#define gzclose fclose
#define gzeof feof_unlocked
#define gzerror(fp, ret) (*(ret)=ferror(fp)?-1:0,"error")
#define gzgetc fgetc_unlocked
#define gzputc(fp, c) fputc_unlocked(c, fp)
#define gzputs(fp, s) fputs_unlocked(s, fp)
#define gzread(fp, buf, size) fread_unlocked(buf, 1, size, fp)
#define gzwrite(fp, buf, size) fwrite_unlocked(buf, 1, size, fp)
#define gzsetparams(fp, a, b) (void)0
#define Z_OK 0
#define Z_STREAM_END 1
