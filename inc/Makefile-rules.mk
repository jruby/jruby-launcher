include inc/Makefile-conf.mk

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE} -f inc/Makefile-rules.mk $(PROGRAM)

jruby.dll: ${OBJECTFILES}
	${LINK.cc} -shared -s -o $@ $^ $(LDLIBSOPTIONS)

jruby: ${OBJECTFILES}
	${LINK.cc} -o $@ $^ $(LDLIBSOPTIONS)

$(OBJECTDIR)/%.o: %.cpp inc/Makefile-rules.mk inc/Makefile-conf.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) $< -MMD -MP -MF $@.d -o $@

$(OBJECTDIR)/%.o: %.c inc/Makefile-rules.mk inc/Makefile-conf.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.c) $< -MMD -MP -MF $@.d -o $@

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/$(CONF)
	${RM} -f $(PROGRAM)

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
