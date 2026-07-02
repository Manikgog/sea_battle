cd /home/mak/Sea_battle/sea_battle

# Создайте правильную структуру AppDir
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/lib
mkdir -p AppDir/usr/qml

# Скопируйте исполняемый файл
cp build/appsea_battle_combined AppDir/usr/bin/

# Создайте .desktop файл
cat > AppDir/sea_battle.desktop << 'EOF'
[Desktop Entry]
Name=Sea Battle
Comment=Sea Battle Game
Exec=appsea_battle_combined
Icon=sea_battle
Terminal=true
Type=Application
Categories=Game;
EOF

# Создайте иконку
convert -size 256x256 xc:blue sea_battle.png 2>/dev/null || echo "Используйте любую иконку"
cp sea_battle.png AppDir/ 2>/dev/null || touch AppDir/sea_battle.png

# Создайте AppRun скрипт
cat > AppDir/AppRun << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin/:${HERE}/usr/sbin/:${HERE}/bin/:${HERE}/sbin/:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib/:${HERE}/usr/lib/x86_64-linux-gnu/:${LD_LIBRARY_PATH}"
export QML_IMPORT_PATH="${HERE}/usr/qml:${QML_IMPORT_PATH}"
export QT_PLUGIN_PATH="${HERE}/usr/plugins:${QT_PLUGIN_PATH}"
exec "${HERE}/usr/bin/appsea_battle_combined" "$@"
EOF

chmod +x AppDir/AppRun

# Пересоберите AppImage
ARCH=x86_64 ./appimagetool-x86_64.AppImage AppDir sea_battle_new.AppImage

# Запустите новый
chmod +x sea_battle_new.AppImage
./sea_battle_new.AppImage
