#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <utf/utfMain.h>

static GT_32 foo(GT_32 idx)
{
    return idx*idx;
}

UTF_TEST_CASE_MAC(foo)
{
    GT_32 idx;
    GT_32 res;

    idx = 45;
    res = foo(idx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(1, res, idx);

    idx = 0xF2;
    res = foo(idx);
    UTF_VERIFY_EQUAL1_PARAM_MAC(1, res, idx);

    idx = 100;
    UTF_VERIFY_EQUAL1_STRING_MAC(1, res, "failed for index [%i]", idx);
}

UTF_TEST_CASE_MAC(TestTwo)
{
    GT_32 i;

    i = 0;
    UTF_VERIFY_EQUAL0_STRING_MAC(0, i, "");
}

UTF_SUIT_BEGIN_TESTS_MAC(TestSuit)
    UTF_SUIT_DECLARE_TEST_MAC(foo)
    UTF_SUIT_DECLARE_TEST_MAC(TestTwo)
UTF_SUIT_END_TESTS_MAC(TestSuit)
