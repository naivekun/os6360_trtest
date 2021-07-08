#

. $tool_build/product/cpss_common.inc
export LINUX_CPSS_TARGET=CPSS_ENABLER
if [ "x$STATIC_LINKED_APPDEMO" = "xYES" ]; then
    PROJECT_DEFS="$PROJECT_DEFS STATIC_LINKED_APPDEMO"
fi
