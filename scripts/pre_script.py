Import("env")
import os

def get_app_name():
  try:
    custom_app = env.GetProjectOption("custom_app", None)
    if custom_app:
      print(f"Using custom app: {custom_app}")
      return custom_app
  except:
    pass
  return env["PIOENV"]

# https://github.com/platformio/platformio-core/blob/4850c1069c67d7074e838238fe1b0316cc7127b1/platformio/builder/main.py#L116
def update_project_paths():
  app_name = get_app_name()
  # define base paths
  lib_dir = f"apps/{app_name}/lib"
  base_paths = {
    'PROJECT_SRC_DIR': f"apps/{app_name}/src",
    'PROJECT_DATA_DIR': f"apps/{app_name}/data",
    'PROJECT_INCLUDE_DIR': f"apps/{app_name}/include",
    'PROJECT_TEST_DIR': f"apps/{app_name}/test",
  }
  # root project directory
  project_dir = env.get("PROJECT_DIR")
  # include lib directory
  env.Prepend(LIBSOURCE_DIRS=[os.path.join(project_dir, lib_dir)])
  # update env variables
  for key, relative_path in base_paths.items():
    full_path = os.path.join(project_dir, relative_path)
    if os.path.exists(full_path):
      env[key] = full_path
      print(f"Updated {key}: {relative_path}")

# execute
update_project_paths()
