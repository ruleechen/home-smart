const yargs = require("yargs");
const del = require("del");
const gulp = require("gulp");
const gzip = require("gulp-gzip");
const uglify = require("gulp-uglify");
const cleanCSS = require("gulp-clean-css");
const rename = require("gulp-rename");

const {
  argv: { buildEnv },
} = yargs.string("buildEnv");

const isRelease = buildEnv === "release";

const webResource = {
  css: ["web/*.css"],
  js: ["web/*.js", "!web/*.min.js"],
  copy: ["web/*.htm"],
  copy_gzip: ["web/*.min.js"],
};

const paths = {
  dist: "data/web",
};

function clean() {
  return del([paths.dist], { force: true });
}

function copy() {
  return gulp.src(webResource.copy).pipe(gulp.dest(paths.dist));
}

function copy_gzip() {
  return gulp.src(webResource.copy_gzip).pipe(gzip()).pipe(gulp.dest(paths.dist));
}

function styles() {
  let stream = gulp.src(webResource.css);
  if (isRelease) {
    stream = stream.pipe(cleanCSS());
  }
  return stream
    .pipe(
      rename({
        suffix: ".min",
      })
    )
    .pipe(gzip())
    .pipe(gulp.dest(paths.dist));
}

function scripts() {
  let stream = gulp.src(webResource.js);
  if (isRelease) {
    stream = stream.pipe(uglify());
  }
  return stream
    .pipe(
      rename({
        suffix: ".min",
      })
    )
    .pipe(gzip())
    .pipe(gulp.dest(paths.dist));
}

function watch() {
  gulp.watch(webResource.css, styles);
  gulp.watch(webResource.js, scripts);
}

const build = gulp.series(
  clean,
  gulp.parallel(copy, copy_gzip, styles, scripts)
);

// export gulp tasks
exports.clean = clean;
exports.styles = styles;
exports.scripts = scripts;
exports.watchWeb = watch;
exports.buildWeb = build;
exports.default = build;
