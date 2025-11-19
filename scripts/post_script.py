Import("env")

# access to global construction environment
# <SCons.Script.SConscript.SConsEnvironment>
# print(env.Dump())

def before_build_littlefs(source, target, env):
  custom_package = env.GetProjectOption("custom_package", "")
  custom_varietas = env.GetProjectOption("custom_varietas", "")
  env.Execute("yarn && yarn build-fs --pioEnv " + env["PIOENV"] + " --custom_package " + custom_package + " --custom_varietas " + custom_varietas)

# Custom actions for specific files/objects
env.AddPreAction("$BUILD_DIR/littlefs.bin", before_build_littlefs)
