// ///////////////////////////////////////////////////////// Factory function //
#define SCRIPT_FACTORY(T)                                                     \
    extern "C" SCRIPTS_API void create##T##(std::shared_ptr<Script> & script, \
                                            Entity entity) {                  \
        script = std::make_shared<T>(entity);                                 \
    }

// ////////////////////////////////////////////////////////////////////////// //
