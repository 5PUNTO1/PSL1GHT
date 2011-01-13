PREFIX		:= ppu-
OBJCOPY		:= $(PREFIX)objcopy
AR			:= $(PREFIX)ar
AS			:= $(PREFIX)gcc
CC			:= $(PREFIX)gcc
CXX			:= $(PREFIX)g++
LD			:= $(CC)
STRIP		:= $(PREFIX)strip
RAW2H		:= $(PSL1GHT)/bin/raw2h
FSELF		:= $(PSL1GHT)/bin/fself.py
SFO			:= $(PSL1GHT)/bin/sfo.py
PS3LOADAPP	:= $(PSL1GHT)/bin/ps3load
SFOXML		:= $(PSL1GHT)/bin/sfo.xml
ICON0		:= $(PSL1GHT)/bin/ICON0.PNG
PKG			:= $(PSL1GHT)/bin/pkg.py
SPRX		:= $(PSL1GHT)/bin/sprxlinker
VPCOMP		:= $(PSL1GHT)/bin/vpcomp

CFLAGS		:= -g \
			   -I$(PSL1GHT)/include -I$(PS3DEV)/host/ppu/include
CXXFLAGS	:= $(CFLAGS)
LDFLAGS		:= -B$(PSL1GHT)/lib -B$(PS3DEV)/host/ppu/lib -llv2 -lpsl1ght

DEPSOPTIONS	=	-MMD -MP -MF $(DEPSDIR)/$*.d

%.o: %.c
	@echo "[CC]  $(notdir $<)"
	@$(CC) $(DEPSOPTIONS) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	@echo "[CXX] $(notdir $<)"
	@$(CXX) $(DEPSOPTIONS) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.S
	@echo "[CC]  $(notdir $<)"
	@$(AS) $(DEPSOPTIONS) -x assembler-with-cpp $(ASFLAGS) $(INCLUDES) -c $< -o $@

%.a:
	@echo "[AR]  $(notdir $@)"
	@$(AR) -rcs $@ $^

%.elf:
	@echo "[LD]  $(notdir $@)"
	@$(LD) $^ $(LIBPATHS) $(LIBS) $(LDFLAGS) -o $@

%.self: %.elf
	@$(STRIP) $< -o $(BUILDDIR)/$(notdir $<)
	@$(SPRX) $(BUILDDIR)/$(notdir $<)
	@$(FSELF) $(BUILDDIR)/$(notdir $<) $@

%.bin.h: %.bin
	@echo "[R2H] $(notdir $<)"
	@$(RAW2H) $< $(BUILDDIR)/$(notdir $<).h $(BUILDDIR)/$(notdir $<).S $(notdir $(basename $<)_bin)
	@$(AS) -x assembler-with-cpp $(ASFLAGS) -c $(BUILDDIR)/$(notdir $<).S -o $(BUILDDIR)/$(notdir $<).o

%.vcg.h: %.vcg
	@echo "[VPCOMP] $(notdir $<)"
	@$(VPCOMP) $< $(notdir $(BUILDDIR)/$(basename $<).rvp)
	@$(RAW2H)  $(BUILDDIR)/$(notdir $(basename $<).rvp) $(BUILDDIR)/$(notdir $<).h $(BUILDDIR)/$(notdir $<).S $(notdir $(basename $<)_bin)
	@$(AS) -x assembler-with-cpp $(ASFLAGS) -c $(BUILDDIR)/$(notdir $<).S -o $(BUILDDIR)/$(notdir $<).o


