Import("env")

# access to global construction environment
# <SCons.Script.SConscript.SConsEnvironment>
# print(env.Dump())

def before_build_littlefs(source, target, env):
  varietas = env.GetProjectOption("custom_varietas", "")
  env.Execute("yarn && yarn build --pioEnv " + env["PIOENV"] + " --varietas " + varietas)

# Custom actions for specific files/objects
env.AddPreAction("$BUILD_DIR/littlefs.bin", before_build_littlefs)
