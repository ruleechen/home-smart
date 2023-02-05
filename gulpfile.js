const yargs = require("yargs");
const path = require("path");
const fse = require("fs-extra");
const del = require("del");
const gulp = require("gulp");
const ignore = require("ignore");

// argv
const {
  argv: { pioEnv },
} = yargs.string("pioEnv");

// paths
const DIR_PACKAGES = "packages";
const DIR_DATA = "data";

const PATH_DEPS = path.resolve(__dirname, ".pio/libdeps", pioEnv);
const PATH_PROJ = path.resolve(__dirname, DIR_PACKAGES, pioEnv);

const PATH_DATA = path.resolve(PATH_PROJ, DIR_DATA);
const PATH_DATA_WEB = path.resolve(PATH_PROJ, DIR_DATA, "web");
const PATH_MAIN = path.resolve(PATH_PROJ, "src/main.cpp");

console.log(`********************
* [env:${pioEnv}]
*
* deps ${PATH_DEPS}
* proj ${PATH_PROJ}
* main ${PATH_MAIN}
*
********************`);

// ignores
const FILE_GITIGNORE = ".gitignore";
const PATH_ROOT_IG = path.join(__dirname, FILE_GITIGNORE);
const ig = ignore();
ig.add(fse.readFileSync(PATH_ROOT_IG).toString());

function clean() {
  return del([PATH_DATA_WEB], { force: true });
}

async function buildDeps() {
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

const build = gulp.series(clean, buildDeps);

// export gulp tasks
exports.build = build;
exports.default = build;
