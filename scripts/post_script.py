Import("env")

# access to global construction environment
# <SCons.Script.SConscript.SConsEnvironment>
# print(env.Dump())

def before_build_littlefs(source, target, env):
  custom_app = env.GetProjectOption("custom_app", "")
  custom_variant = env.GetProjectOption("custom_variant", "")
  env.Execute("yarn && yarn build-fs --pioEnv " + env["PIOENV"] + " --custom_app " + custom_app + " --custom_variant " + custom_variant)

# Custom actions for specific files/objects
env.AddPreAction("$BUILD_DIR/littlefs.bin", before_build_littlefs)
