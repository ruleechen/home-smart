const yargs = require("yargs");
const path = require("path");
const fse = require("fs-extra");
const del = require("del");
const gulp = require("gulp");
const ignore = require("ignore");

const PATH_ROOT = path.resolve(__dirname, "./");
const PATH_DEPS = path.resolve(__dirname, "./.pio/libdeps");

const DIR_DATA = "data";
const DIR_DATA_WEB = path.resolve(DIR_DATA, "web");

const ig = ignore();
ig.add(fse.readFileSync(path.join(__dirname, ".gitignore")).toString());

const {
  argv: { pioEnv },
} = yargs.string("pioEnv");

function clean() {
  return del([DIR_DATA_WEB], { force: true });
}

function buildDeps() {
  if (fse.existsSync(PATH_DEPS)) {
    const libDir = path.resolve(PATH_DEPS, pioEnv, "home-esp8266", DIR_DATA);
    if (fse.existsSync(libDir)) {
      const destination = path.resolve(PATH_ROOT, "packages", pioEnv, DIR_DATA);
      fse.copySync(libDir, destination, {
        overwrite: true,
        filter: (src, dest) => {
          const relDest = path.relative(__dirname, dest);
          return relDest === DIR_DATA || ig.ignores(relDest);
        },
      });
    }
  }
  return Promise.resolve();
}

const build = gulp.series(clean, buildDeps);

// export gulp tasks
exports.build = build;
exports.default = build;
