#!/usr/bin/env bash
# Sends a firmware image to a gadget over Wi-Fi through its Hermes gateway, from any computer.
# See docs/setup-board.md#manage-an-existing-gadget.
set -euo pipefail

host="${HERMES_SSH_HOST:-}"
container="${HERMES_CONTAINER:-}"
use_sudo=0
dry_run=0
no_wait=0
container_user=10000
container_home=/opt/data
hermes_bin=""

usage() {
  cat <<'EOF'
Usage: gadget-ota.sh [options] <device> <firmware.bin>

  <device>         the gadget's name or ID (see: hermes gadget devices)
  <firmware.bin>   the app image, not a merged image

Options:
  --host [USER@]HOST   Hermes runs on this machine, reached over SSH ($HERMES_SSH_HOST).
                       Without it, Hermes runs on this computer.
  --container NAME     Hermes runs in this Docker container ($HERMES_CONTAINER)
  --sudo               run on the Hermes machine with sudo; asks for your password
  --container-user ID  user inside the container (default: 10000)
  --hermes-data DIR    Hermes's data directory inside the container (default: /opt/data)
  --hermes-bin PATH    the hermes command (default: hermes, or the image's own in a container)
  --no-wait            stage the update and return; the gateway installs it when the device is online
  --dry-run            print what would run and change nothing
  -h, --help           show this help

Examples:
  gadget-ota.sh "Kitchen" build/hermes_gadget.bin
  gadget-ota.sh --host me@nas --container hermes --sudo "Kitchen" build/hermes_gadget.bin
EOF
}

die() {
  echo "gadget-ota: $*" >&2
  exit 2
}

value() {
  [ "$#" -ge 2 ] || die "$1 needs a value"
}

args=()
while [ "$#" -gt 0 ]; do
  case "$1" in
    --host) value "$@"; host="$2"; shift 2 ;;
    --container) value "$@"; container="$2"; shift 2 ;;
    --container-user) value "$@"; container_user="$2"; shift 2 ;;
    --hermes-data) value "$@"; container_home="$2"; shift 2 ;;
    --hermes-bin) value "$@"; hermes_bin="$2"; shift 2 ;;
    --sudo) use_sudo=1; shift ;;
    --no-wait) no_wait=1; shift ;;
    --dry-run) dry_run=1; shift ;;
    -h|--help) usage; exit 0 ;;
    --) shift; args+=("$@"); break ;;
    -*) die "unknown option $1 (try --help)" ;;
    *) args+=("$1"); shift ;;
  esac
done

[ "${#args[@]}" -eq 2 ] || { usage >&2; exit 2; }
device="${args[0]}"
image="${args[1]}"
[ -f "$image" ] || die "no firmware image at $image"

# Quotes one word for a POSIX shell.
quote() {
  printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"
}

stamp="hermes-gadget-ota-$$-$RANDOM.bin"
image_there="$image"
[ -z "$host" ] || image_there="/tmp/$stamp"

update_flags=""
[ "$no_wait" -eq 0 ] || update_flags=" --no-wait"

if [ -n "$container" ]; then
  [ -n "$hermes_bin" ] || hermes_bin=/opt/hermes/.venv/bin/hermes
  c="$(quote "$container")"
  inside="/tmp/$stamp"
  run_as="-u $(quote "$container_user") -e HOME=$(quote "$container_home/home") -e HERMES_HOME=$(quote "$container_home")"
  script="docker cp $(quote "$image_there") $c:$inside && docker exec -u 0 $c chmod 644 $inside"
  script+=" && docker exec $run_as $c $(quote "$hermes_bin") gadget update $(quote "$device") $inside$update_flags"
  script+="; status=\$?; docker exec -u 0 $c rm -f $inside"
else
  [ -n "$hermes_bin" ] || hermes_bin=hermes
  script="$(quote "$hermes_bin") gadget update $(quote "$device") $(quote "$image_there")$update_flags; status=\$?"
fi
[ -z "$host" ] || script+="; rm -f $(quote "$image_there")"
script+="; exit \$status"

command="$script"
[ "$use_sudo" -eq 0 ] || command="sudo sh -c $(quote "$script")"

if [ "$dry_run" -eq 1 ]; then
  [ -z "$host" ] || echo "scp $image $host:$image_there"
  if [ -n "$host" ]; then echo "ssh -t $host $command"; else echo "sh -c $command"; fi
  exit 0
fi

if [ -n "$host" ]; then
  echo "Uploading $image to $host..."
  scp -q "$image" "$host:$image_there"
  [ "$use_sudo" -eq 0 ] || echo "Type your sudo password. Nothing shows while you type; that is normal."
  ssh -t "$host" "$command"
else
  sh -c "$command"
fi
