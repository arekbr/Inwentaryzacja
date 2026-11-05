#!/bin/bash
# Import tożsamości podpisu kodu macOS z sekretów GitHuba do tymczasowego pęku kluczy runnera.
#
# Po co: podpis ad hoc opiera wymaganie podpisu na skrócie binarki (cdhash). Od 5.11.2026 sekrety
# zawierają Developer ID Application z pośrednikiem G2, co daje Team ID i wymaganie stałe między
# wersjami. Do 5.11.2026 była tu tożsamość własna, przy której pytanie Pęku kluczy po aktualizacji
# zostawało (pomiar 23.10.2026: partition_id bez Team ID trzyma `cdhash:…`).
# „Zero pytań z Team ID” to nadal hipoteza do zmierzenia na dwóch kolejnych wydaniach.
#
# Wejście (env): MACOS_CODESIGN_P12_BASE64, MACOS_CODESIGN_P12_PASSWORD, opcjonalnie REQUIRE_CODESIGN=1.
# Wyjście: MACOS_CODESIGN_IDENTITY w $GITHUB_ENV (czyta go package-macos.sh).
# Bez sekretu (PR z forka, Dependabot) nic nie robi — paczka dostanie podpis ad hoc,
# chyba że REQUIRE_CODESIGN=1 (tag wydania): wtedy błąd zamiast cichego ad hoc.
set -euo pipefail

if [[ -z "${MACOS_CODESIGN_P12_BASE64:-}" ]]; then
    if [[ "${REQUIRE_CODESIGN:-0}" == "1" ]]; then
        echo "❌ Brak sekretu MACOS_CODESIGN_P12_BASE64 — wydanie wymaga stałego podpisu."
        exit 1
    fi
    echo "⚠ Brak sekretu podpisu — paczka zostanie podpisana ad hoc."
    exit 0
fi

KEYCHAIN="$RUNNER_TEMP/podpis-kodu.keychain-db"
KEYCHAIN_PASSWORD="$(openssl rand -hex 24)"
P12="$RUNNER_TEMP/podpis-kodu.p12"
trap 'rm -f "$P12"' EXIT

printf '%s' "$MACOS_CODESIGN_P12_BASE64" | base64 --decode > "$P12"
security create-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN"
security set-keychain-settings -lut 21600 "$KEYCHAIN"
security unlock-keychain -p "$KEYCHAIN_PASSWORD" "$KEYCHAIN"
security import "$P12" -k "$KEYCHAIN" -P "$MACOS_CODESIGN_P12_PASSWORD" -T /usr/bin/codesign
# Bez listy partycji codesign czekałby na zgodę w oknie, którego na runnerze nikt nie kliknie.
security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$KEYCHAIN_PASSWORD" "$KEYCHAIN" >/dev/null
# codesign szuka tożsamości tylko w pękach z listy wyszukiwania — samo --keychain nie wystarcza
# (pomiar 23.10.2026: „no identity found” bez wpisu na liście).
SEARCH_LIST=()
while IFS= read -r entry; do
    entry="${entry#"${entry%%[![:space:]]*}"}"  # wcięcie z wydruku `security`
    SEARCH_LIST+=("${entry//\"/}")
done < <(security list-keychains -d user)
security list-keychains -d user -s "$KEYCHAIN" "${SEARCH_LIST[@]}"

IDENTITY="$(security find-identity -p codesigning "$KEYCHAIN" | awk '$1 == "1)" {print $2; exit}')"
if [[ -z "$IDENTITY" ]]; then
    echo "❌ Plik p12 zaimportowany, ale pęk nie ma tożsamości do podpisu kodu."
    exit 1
fi
echo "MACOS_CODESIGN_IDENTITY=$IDENTITY" >> "$GITHUB_ENV"
echo "✓ Tożsamość podpisu: $IDENTITY"
