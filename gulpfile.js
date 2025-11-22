const cp = require("child_process");
const yargs = require("yargs");
const path = require("path");
const fse = require("fs-extra");
const del = require("del");
const ignore = require("ignore");

// dirs
const DIR_APPS = "apps";
const DIR_DATA = "data";
const DIR_VARIANT = "variant";
const DIR_DEFAULT = ".default";

// argv
const {
  argv: {
    pioEnv,
    custom_app,
    custom_variant
  },
} = yargs
  .string("pioEnv")
  .string("custom_app")
  .string("custom_variant");
const appName = custom_app || pioEnv;
const variant = custom_variant || DIR_DEFAULT;

// paths
const PATH_DEPS = path.resolve(__dirname, ".pio/libdeps", pioEnv);
const PATH_PROJ = path.resolve(__dirname, DIR_APPS, appName);

const PATH_DATA = path.resolve(PATH_PROJ, DIR_DATA);
const PATH_DATA_WEB = path.resolve(PATH_PROJ, DIR_DATA, "web");
const PATH_MAIN = path.resolve(PATH_PROJ, "src/main.cpp");

console.log(`********************
*
* [env:${pioEnv}]
*
* deps: '${PATH_DEPS}'
* proj: '${PATH_PROJ}'
* main: '${PATH_MAIN}'
* app: '${appName}'
* variant: '${variant}'
*
********************`);

// ignores
const FILE_GITIGNORE = ".gitignore";
const PATH_ROOT_IG = path.join(__dirname, FILE_GITIGNORE);
const ig = ignore();
ig.add(fse.readFileSync(PATH_ROOT_IG).toString());

function clean() {
  if (pioEnv === "home-esp8266") {
    del.sync([PATH_DATA_WEB], { force: true });
  } else {
    del.sync([PATH_DATA], { force: true });
  }
}

function buildDeps() {
  if (fse.existsSync(PATH_DEPS)) {
    const libDir = path.resolve(PATH_DEPS, "home-esp8266", DIR_DATA);
    if (fse.existsSync(libDir)) {
      const refDataPath = path.relative(__dirname, PATH_DATA);
      fse.copySync(libDir, PATH_DATA, {
        overwrite: true,
        filter: (src, dest) => {
          const relDest = path.relative(__dirname, dest);
          return relDest === refDataPath || ig.ignores(relDest);
        },
      });
    }
  }
}

function mergeVariant() {
  const variantPath = path.resolve(PATH_PROJ, DIR_VARIANT, variant);
  if (fse.pathExistsSync(variantPath)) {
    // merge configuration
    const variantDataPath = path.resolve(variantPath, DIR_DATA);
    if (fse.pathExistsSync(variantDataPath)) {
      fse.copySync(variantDataPath, PATH_DATA, {
        overwrite: true,
      });
    }
  }
}

async function buildFS() {
  if (pioEnv === "home-esp8266") {
    cp.execSync(`yarn workspace ${pioEnv} build-web --buildEnv release`, {
      stdio: "inherit",
    });
  } else {
    clean();
    buildDeps();
    mergeVariant();
  }
}

exports.buildFS = buildFS;
