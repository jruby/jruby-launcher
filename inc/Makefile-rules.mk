include inc/Makefile-conf.mk

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE} -f inc/Makefile-rules.mk jruby.dll

jruby.dll: ${OBJECTFILES}
	${LINK.cc} -shared -o jruby.dll -s ${OBJECTFILES} ${LDLIBSOPTIONS} 

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
	${RM} -f jruby.dll

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
