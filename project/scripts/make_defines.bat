@echo -------------------------------------------------
@rem (this is for asm-files)
@echo Generating include-file for asm from defines.h
python .\scripts\generateIncFile.py src/include/defines.h src/include_gen/defines.inc

@rem expose defines for the glass-assembler too.
python .\scripts\generateExcerptGlassIncFile.py src/include_gen/defines.inc src/include_gen/defines.glass.inc

