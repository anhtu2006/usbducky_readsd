#ifndef WEB_INDEX_H
#define WEB_INDEX_H

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Duck IDE</title>
    <link rel="stylesheet" href="/web/style.min.css" />
    <script src="/web/jquery.min.js"></script>
    <script src="/web/jstree.min.js"></script>
    <style>
        * { box-sizing: border-box; }
        body { background: #1e1e1e; color: #d4d4d4; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; display: flex; height: 100vh; margin: 0; overflow: hidden; }
        #sidebar { width: 280px; border-right: 1px solid #333; overflow: auto; background: #252526; padding: 10px; }
        #editor-area { flex: 1; display: flex; flex-direction: column; background: #1e1e1e; }
        #toolbar { padding: 8px 15px; background: #333; display: flex; gap: 10px; align-items: center; border-bottom: 1px solid #444; }
        textarea { flex: 1; background: #1e1e1e; color: #9cdcfe; border: none; padding: 20px; font-family: 'Consolas', monospace; font-size: 14px; outline: none; resize: none; }
        button { padding: 6px 12px; border: none; border-radius: 3px; cursor: pointer; font-weight: bold; transition: 0.2s; }
        .btn-save { background: #007acc; color: white; }
        .btn-save:hover { background: #005a9e; }
        .btn-run { background: #28a745; color: white; }
        .btn-run:hover { background: #218838; }
        .btn-help { background: #6c757d; color: white; }
        .btn-help:hover { background: #5a6268; }
        #current-file { font-size: 11px; color: #888; flex: 1; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
        .jstree-anchor { color: #ccc !important; font-size: 13px; }
        
        /* CSS Fallback: Tạo cấu trúc cây nếu thiếu file style.min.css của jstree */
        .jstree-container-ul, .jstree-children { padding-left: 20px; list-style: none; }
        .jstree-node { white-space: nowrap; }
        .jstree-icon { margin-right: 5px; vertical-align: middle; display: inline-block; width: 20px; }
        .jstree-ocl { display: inline-block; width: 20px; text-align: center; cursor: pointer; }

        /* Mobile Responsive: Giao diện điện thoại */
        #toggle-sidebar { display: none; background: transparent; border: none; color: #fff; font-size: 24px; cursor: pointer; margin-right: 10px; }
        @media (max-width: 768px) {
            #sidebar { position: fixed; top: 0; left: -100%; width: 85%; height: 100%; z-index: 1000; transition: left 0.3s; box-shadow: 2px 0 10px rgba(0,0,0,0.5); }
            #sidebar.open { left: 0; }
            #toggle-sidebar { display: inline-block; }
            #current-file { display: none; } /* Ẩn đường dẫn để tiết kiệm chỗ */
            button { padding: 10px 15px; font-size: 14px; } /* Nút to hơn cho dễ bấm */
        }

        /* Modal Help */
        .modal { display: none; position: fixed; z-index: 2000; left: 0; top: 0; width: 100%; height: 100%; overflow: auto; background-color: rgba(0,0,0,0.7); }
        .modal-content { background-color: #252526; margin: 10% auto; padding: 20px; border: 1px solid #444; width: 80%; max-width: 600px; color: #d4d4d4; border-radius: 5px; box-shadow: 0 4px 8px rgba(0,0,0,0.5); }
        .close { color: #aaa; float: right; font-size: 28px; font-weight: bold; cursor: pointer; }
        .close:hover, .close:focus { color: #fff; text-decoration: none; cursor: pointer; }
        .modal h2 { border-bottom: 1px solid #444; padding-bottom: 10px; margin-top: 0; font-size: 18px; }
        .modal ul { line-height: 1.6; padding-left: 20px; }
        .modal code { background: #333; padding: 2px 5px; border-radius: 3px; font-family: 'Consolas', monospace; color: #9cdcfe; font-size: 13px; }
    </style>
</head>
<body>
    <div id="sidebar">
        <div style="padding-bottom:10px; border-bottom:1px solid #444; margin-bottom:10px;">
            <input type="text" id="new-path" placeholder="Đường dẫn (vd: /)" value="/" style="width:100%; margin-bottom:5px; background:#333; color:#ccc; border:1px solid #555; padding:5px;">
            <input type="text" id="new-name" placeholder="Tên mới (vd: script.txt)" style="width:100%; margin-bottom:5px; background:#333; color:#ccc; border:1px solid #555; padding:5px;">
            <div style="display:flex; gap: 5px;">
                <select id="new-type" style="flex:1; background:#333; color:#ccc; border:1px solid #555; padding:5px;"><option value="file">File</option><option value="folder">Folder</option></select>
                <button onclick="createItem()" style="flex:1; background:#007acc; color:white; border:none; padding:5px; cursor:pointer;">Tạo</button>
            </div>
        </div>
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom: 10px; padding: 0 2px;">
            <h4 style="color:#888; margin:0; font-size: 11px; font-weight:bold; letter-spacing: 1px;">EXPLORER</h4>
            <button onclick="renameItem()" style="background:none; border:none; cursor:pointer; color:#ccc; font-size:14px; padding:0;" title="Đổi tên">✏️</button>
        </div>
        <div id="jstree_files"></div>
    </div>
    <div id="editor-area">
        <div id="toolbar">
            <button id="toggle-sidebar" onclick="toggleSidebar()">☰</button>
            <span id="current-file">Không có file nào được chọn</span>
            <button class="btn-save" onclick="saveFile()">💾 LƯU</button>
            <button class="btn-run" onclick="runFile()">🚀 CHẠY</button>
            <button class="btn-help" onclick="openHelp()">?</button>
        </div>
        <textarea id="editor" spellcheck="false" placeholder="Nội dung script..."></textarea>
    </div>

    <!-- Help Modal -->
    <div id="helpModal" class="modal">
        <div class="modal-content">
            <span class="close" onclick="closeHelp()">&times;</span>
            <h2>Ducky Script Basics</h2>
            <ul>
                <li><code>REM</code>: Ghi chú (Comment).</li>
                <li><code>DEFAULT_DELAY [ms]</code>: Thời gian chờ mặc định giữa các lệnh.</li>
                <li><code>DELAY [ms]</code>: Chờ một khoảng thời gian (mili giây).</li>
                <li><code>STRING [text]</code>: Gõ đoạn văn bản.</li>
                <li><code>REPEAT [n]</code>: Lặp lại lệnh trước đó n lần.</li>
                <li><strong>Phím đặc biệt:</strong> <code>ENTER</code>, <code>TAB</code>, <code>ESC</code>, <code>BACKSPACE</code>, <code>DELETE</code>, <code>INSERT</code>, <code>HOME</code>, <code>END</code>, <code>PAGEUP</code>, <code>PAGEDOWN</code>, <code>UP</code>, <code>DOWN</code>, <code>LEFT</code>, <code>RIGHT</code>, <code>F1</code>-<code>F12</code>.</li>
                <li><strong>Phím tổ hợp:</strong> <code>CTRL</code>, <code>ALT</code>, <code>SHIFT</code>, <code>GUI</code> (Windows). <br>Ví dụ: <code>CTRL ALT DELETE</code>, <code>GUI r</code>.</li>
            </ul>
        </div>
    </div>

    <script>
        let selectedPath = "";
        $(function () {
            $('#jstree_files').jstree({
                'core': { 
                    'data': { 'url': '/tree', 'data': n => ({'id': n.id}) },
                    'themes': { 'name': 'default', 'dots': true, 'icons': true }
                },
                'types': { 'file': { 'icon': 'jstree-file' }, 'default': { 'icon': 'jstree-folder' } },
                'plugins': ['types']
            }).on("select_node.jstree", function (e, data) {
                // Tự động điền đường dẫn vào ô tạo mới
                let nodePath = data.node.id;
                if (data.node.original.type === 'default') $('#new-path').val(nodePath);
                else {
                    let parent = nodePath.substring(0, nodePath.lastIndexOf('/'));
                    $('#new-path').val(parent || '/');
                }

                if(data.node.original.type === 'file') {
                    selectedPath = data.node.id;
                    $('#current-file').text(selectedPath);
                    fetch('/get-content?path=' + encodeURIComponent(selectedPath))
                        .then(r => r.text()).then(t => $('#editor').val(t));
                    
                    // Mobile: Tự động đóng menu khi chọn xong file
                    if(window.innerWidth < 768) toggleSidebar();
                }
            });
        });

        function saveFile() {
            if(!selectedPath) return alert("Vui lòng chọn file!");
            let fd = new FormData();
            fd.append('path', selectedPath);
            fd.append('content', $('#editor').val());
            fetch('/save', { method: 'POST', body: fd })
                .then(r => r.text()).then(msg => alert(msg));
        }

        function runFile() {
            if(!selectedPath) return alert("Vui lòng chọn file!");
            fetch('/run?f=' + encodeURIComponent(selectedPath));
        }

        function createItem() {
            let path = $('#new-path').val();
            let name = $('#new-name').val();
            let type = $('#new-type').val();
            if(!name) return alert("Vui lòng nhập tên!");

            let fd = new FormData();
            fd.append('path', path); fd.append('name', name); fd.append('type', type);
            
            fetch('/create', { method: 'POST', body: fd }).then(r => {
                if(r.ok) {
                    $('#jstree_files').jstree(true).refresh();
                    if(type === 'file') {
                        selectedPath = (path.endsWith('/') ? path : path + '/') + name;
                        selectedPath = selectedPath.replace('//', '/');
                        $('#current-file').text(selectedPath);
                        $('#editor').val(""); // File mới thì trống
                    }
                } else r.text().then(t => alert("Lỗi: " + t));
            });
        }

        function renameItem() {
            let ref = $('#jstree_files').jstree(true);
            let sel = ref.get_selected();
            if(!sel.length) return alert("Vui lòng chọn file/folder cần đổi tên!");
            
            let oldPath = sel[0];
            let oldName = ref.get_text(oldPath);
            let newName = prompt("Nhập tên mới:", oldName);
            
            if(!newName || newName === oldName) return;
            
            let parent = oldPath.substring(0, oldPath.lastIndexOf('/'));
            let newPath = (parent === "" ? "" : parent) + "/" + newName;
            
            let fd = new FormData();
            fd.append('old', oldPath); fd.append('new', newPath);
            
            fetch('/rename', { method: 'POST', body: fd }).then(r => {
                if(r.ok) { ref.refresh(); } else r.text().then(t => alert("Lỗi: " + t));
            });
        }

        function toggleSidebar() {
            $('#sidebar').toggleClass('open');
        }

        function openHelp() { document.getElementById('helpModal').style.display = 'block'; }
        function closeHelp() { document.getElementById('helpModal').style.display = 'none'; }
        
        window.onclick = function(event) {
            if (event.target == document.getElementById('helpModal')) {
                closeHelp();
            }
        }
    </script>
</body>
</html>
)=====";

#endif