CC = gcc
CFLAGS = -O3 -Wextra -std=c99 -Iinclude -Iinclude/SDL3
LDFLAGS = -Llibs/SDL3 -lSDL3 -lm

SRCDIR = src
INCDIR = include
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
BINDIR = $(BUILDDIR)/bin

SOURCES = $(filter-out $(SRCDIR)/SDL3.dll, $(wildcard $(SRCDIR)/*.c))
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/physics_engine.exe

all: $(TARGET) copy_dll

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

copy_dll: $(TARGET)
	copy $(SRCDIR)\SDL3.dll $(BINDIR)\

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"

$(BINDIR):
	if not exist "$(BINDIR)" mkdir "$(BINDIR)"

clean:
	rmdir /s /q $(BUILDDIR)

run: $(TARGET) copy_dll
	cd $(BINDIR) && physics_engine.exe

.PHONY: all clean run copy_dll