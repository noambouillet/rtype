#!/usr/bin/env bash
# Applies the degraded network conditions of the project annex to the loopback
# interface with Linux netem, so the real client and server can be played by hand.
# Needs root (tc). Everything on 127.0.0.1 is affected until you run "off".
#
#   sudo tests/functional/netem.sh loss        # 2% packet loss
#   sudo tests/functional/netem.sh duplicate   # 2% duplication
#   sudo tests/functional/netem.sh latency     # 150 ms +/- 30 ms, so packets also get reordered
#   sudo tests/functional/netem.sh bandwidth   # 5 KB/s
#   sudo tests/functional/netem.sh all         # all of the above together
#   sudo tests/functional/netem.sh show        # current rules
#   sudo tests/functional/netem.sh off         # back to a normal loopback

set -euo pipefail

DEVICE="${NETEM_DEVICE:-lo}"

apply() {
    tc qdisc del dev "$DEVICE" root 2>/dev/null || true
    tc qdisc add dev "$DEVICE" root netem "$@"
    echo "netem on $DEVICE: $*"
}

case "${1:-}" in
    loss)      apply loss 2% ;;
    duplicate) apply duplicate 2% ;;
    latency)   apply delay 150ms 30ms distribution normal ;;
    bandwidth) apply rate 40kbit limit 50 ;;
    all)       apply delay 150ms 30ms distribution normal loss 2% duplicate 2% rate 40kbit limit 50 ;;
    show)      tc qdisc show dev "$DEVICE" ;;
    off)       tc qdisc del dev "$DEVICE" root 2>/dev/null || true; echo "netem removed from $DEVICE" ;;
    *)         sed -n '2,13p' "$0"; exit 2 ;;
esac
