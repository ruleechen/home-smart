Import("env")
import os

def get_package_name():
  try:
    custom_package = env.GetProjectOption("custom_package", None)
    if custom_package:
      print(f"Using custom package: {custom_package}")
      return custom_package
  except:
    pass
  return env["PIOENV"]

def update_project_paths():
  package_name = get_package_name()
  # define base paths
  lib_dir = f"packages/{package_name}/lib"
  base_paths = {
    'PROJECT_SRC_DIR': f"packages/{package_name}/src",
    'PROJECT_DATA_DIR': f"packages/{package_name}/data",
    'PROJECT_INCLUDE_DIR': f"packages/{package_name}/include",
    'PROJECT_TEST_DIR': f"packages/{package_name}/test",
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
