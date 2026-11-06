add_test([=[MatchingEngine.BuyOrderMatchesSellOrder]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=MatchingEngine.BuyOrderMatchesSellOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngine.BuyOrderMatchesSellOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/matching_engine_test.cpp:7]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[MatchingEngine.BuyOrderPartiallyExecutes]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=MatchingEngine.BuyOrderPartiallyExecutes]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[MatchingEngine.BuyOrderPartiallyExecutes]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/order_validation_test.cpp:127]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderValidation.AcceptsValidOrder]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=OrderValidation.AcceptsValidOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderValidation.AcceptsValidOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/order_validation_test.cpp:6]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderValidation.RejectsZeroQuantity]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=OrderValidation.RejectsZeroQuantity]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderValidation.RejectsZeroQuantity]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/order_validation_test.cpp:28]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderValidation.RejectsNegativePrice]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=OrderValidation.RejectsNegativePrice]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderValidation.RejectsNegativePrice]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/order_validation_test.cpp:48]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderValidation.RejectsDuplicateOrderID]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=OrderValidation.RejectsDuplicateOrderID]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderValidation.RejectsDuplicateOrderID]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/order_validation_test.cpp:68]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[OrderValidation.RejectsInvalidSide]=]  D:/Study/UG/PROJECTS/HFT_Engine/build/HFT_Tests.exe [==[--gtest_filter=OrderValidation.RejectsInvalidSide]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[OrderValidation.RejectsInvalidSide]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[D:/Study/UG/PROJECTS/HFT_Engine/tests/order_validation_test.cpp:102]==]
    WORKING_DIRECTORY [==[D:/Study/UG/PROJECTS/HFT_Engine/build]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(HFT_Tests_TESTS [==[MatchingEngine.BuyOrderMatchesSellOrder]==] [==[MatchingEngine.BuyOrderPartiallyExecutes]==] [==[OrderValidation.AcceptsValidOrder]==] [==[OrderValidation.RejectsZeroQuantity]==] [==[OrderValidation.RejectsNegativePrice]==] [==[OrderValidation.RejectsDuplicateOrderID]==] [==[OrderValidation.RejectsInvalidSide]==])
