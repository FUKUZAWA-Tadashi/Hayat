-include ./MACHDEP

ifdef MACHDEP_DIR

include $(MACHDEP_DIR)/Makefile

else

clean:
	$(MAKE) -f ../machdep/unix/Makefile clean

endif

