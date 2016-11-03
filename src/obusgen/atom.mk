
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_HOST_MODULE := obusgen

obusgen_py_files := $(call all-files-under,.,.py)

# Install all python file in host staging directory
LOCAL_COPY_FILES := \
	$(foreach __f,$(obusgen_py_files), \
		$(__f):$(HOST_OUT_STAGING)/usr/lib/obusgen/$(__f) \
	)

# Needed to force a build order of LOCAL_COPY_FILES
LOCAL_EXPORT_PREREQUISITES := \
	$(foreach __f,$(obusgen_py_files), \
		$(HOST_OUT_STAGING)/usr/lib/obusgen/$(__f) \
	)

include $(BUILD_CUSTOM)

###############################################################################
## Custom macro that can be used in LOCAL_CUSTOM_MACROS of a module to
## create automatically rules to generate files from xml.
## Note : in the context of the macro, LOCAL_XXX variables refer to the module
## that use the macro, not this module defining the macro.
## As the content of the macro is 'eval' after, most of variable ref shall be
## escaped (hence the $$). Only $1, $2... variables can be used directly.
## Note : no 'global' variable shall be used except the ones defined by
## alchemy (TARGET_XXX and HOST_XXX variables). Otherwise the macro will no
## work when integrated in a SDK (using local-register-custom-macro).
## Note : rules shoud NOT use any variables defined in the context of the
## macro (for the same reason PRIVATE_XXX variables shall be used in place of
## LOCAL_XXX variables).
## Note : if you need a script or a binary, please install it in host staging
## directory and execute it from there. This way it will also work in the
## context of a SDK.
###############################################################################

# Before obusgen is installed, we need it during makefile parsing phase
# We define this variable to find it in $(LOCAL_PATH) if not found yet in
# host staging directory
obusgen-macro-path := $(LOCAL_PATH)

# $1: client/server
# $2: language (c)
# $3: java package name
# $4: output directory (Relative to build directory unless an absolute path is
#     given (ex LOCAL_PATH).
# $5: input xml file
define obusgen-macro

# Setup some internal variables
obusgen_xml_file := $5
obusgen_module_build_dir := $(call local-get-build-dir)
obusgen_out_dir := $(if $(call is-path-absolute,$4),$4,$$(obusgen_module_build_dir)/$4)
obusgen_done_file := $$(obusgen_module_build_dir)/$$(notdir $$(obusgen_xml_file)).done
$(if $(wildcard $(HOST_OUT_STAGING)/usr/lib/obusgen/obusgen.py), \
	obusgen_gen_files := $$(shell $(HOST_OUT_STAGING)/usr/lib/obusgen/obusgen.py \
		--files --$1 --lang $2 --package $3 -o $$(obusgen_out_dir) $5) \
	, \
	obusgen_gen_files := $$(shell $(obusgen-macro-path)/obusgen.py \
		--files --$1 --lang $2 --package $3 -o $$(obusgen_out_dir) $5) \
)
obusgen_c_files := $$(filter %.c,$$(obusgen_gen_files))
obusgen_h_files := $$(filter %.h,$$(obusgen_gen_files))

# Create a dependency between generated files and .done file with an empty
# command to make sure regeneration is correctly triggered to files
# depending on them
$$(obusgen_gen_files): $$(obusgen_done_file)
	$(empty)

# Force suppression of done file if any of the generated file is missing
# This will force trigerring the generation
$$(foreach __f,$$(obusgen_gen_files), \
	$$(if $$(wildcard $$(__f)),,$$(shell rm -f $$(obusgen_done_file))) \
)

# Actual generation rule
# The copy of xml is staging is done in 2 steps because several modules could use
# the same xml the move ensure atomicity of the copy.
$$(obusgen_done_file): $(addprefix $(HOST_OUT_STAGING)/usr/lib/obusgen/,$(obusgen_py_files))
$$(obusgen_done_file): PRIVATE_OUT_DIR := $$(obusgen_out_dir)
$$(obusgen_done_file): $$(obusgen_xml_file)
	@echo "$$(PRIVATE_MODULE): Generating obus files from $$(call path-from-top,$5)"
	$(Q) $(HOST_OUT_STAGING)/usr/lib/obusgen/obusgen.py \
		--$1 --lang $2 --package $3 -o $$(PRIVATE_OUT_DIR) $5
	@mkdir -p $(TARGET_OUT_STAGING)/usr/share/obus
	$(Q) cp -af $5 $(TARGET_OUT_STAGING)/usr/share/obus/$(notdir $5).$$(PRIVATE_MODULE)
	$(Q) mv -f $(TARGET_OUT_STAGING)/usr/share/obus/$(notdir $5).$$(PRIVATE_MODULE) \
		$(TARGET_OUT_STAGING)/usr/share/obus/$(notdir $5)
	@mkdir -p $$(dir $$@)
	@touch $$@

# Update either LOCAL_SRC_FILES or LOCAL_GENERATED_SRC_FILES
$(if $(call is-path-absolute,$4), \
	LOCAL_SRC_FILES += $$(patsubst $$(LOCAL_PATH)/%,%,$$(obusgen_c_files)) \
	, \
	LOCAL_GENERATED_SRC_FILES += $$(patsubst $$(obusgen_module_build_dir)/%,%,$$(obusgen_c_files)) \
)

# Update alchemy variables for the module
LOCAL_CLEAN_FILES += $$(obusgen_done_file)
LOCAL_PREREQUISITES += $$(obusgen_xml_file)
LOCAL_EXPORT_PREREQUISITES += $$(obusgen_gen_files) $$(obusgen_done_file)
LOCAL_CUSTOM_TARGETS += $$(obusgen_done_file)

endef

# Register the macro in alchemy so it will be integrated in generated sdk
$(call local-register-custom-macro,obusgen-macro)
