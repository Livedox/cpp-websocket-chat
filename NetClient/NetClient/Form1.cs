using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using WebSocketSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;


namespace NetClient
{
    public partial class Form1 : Form
    {
        WebSocket ws;
        List<string> idlist = new List<string>() {  };

        private FlowLayoutPanel CreateMessageContainer(string message, string id)
        {
            var layout = new FlowLayoutPanel();
            layout.AutoSize = true;
            layout.Margin = new Padding(0, 0, 0, 20);
            layout.Padding = new Padding(5);
            layout.BackColor = Color.FromArgb(44, 62, 80);

            var messageLabel = new Label();
            messageLabel.Text = message;
            messageLabel.AutoSize = true;
            messageLabel.ForeColor = Color.FromArgb(236, 240, 241);
            messageLabel.TextAlign = ContentAlignment.MiddleLeft;
            messageLabel.Font = new Font(FontFamily.GenericSansSerif, 16);

            var exitLabel = new Label();
            exitLabel.Padding = new Padding(0, 4, 0, 0);
            exitLabel.Text = "X";
            exitLabel.Cursor = Cursors.Hand;
            exitLabel.AutoSize = true;
            exitLabel.TextAlign = ContentAlignment.MiddleCenter;
            exitLabel.ForeColor = Color.FromArgb(255, 0, 0);
            exitLabel.Font = new Font(FontFamily.GenericSansSerif, 14);
            exitLabel.Click += (object sender, System.EventArgs e) =>
            {
                Ws_Send("{ \"head\":\"delete\", \"_id\":\"" + id + "\"}");
            };
            layout.Controls.Add(messageLabel);
            layout.Controls.Add(exitLabel);

            return layout;
        }
        public Form1()
        {
            ws = new WebSocket("ws://127.0.0.1:8083");
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ws.OnMessage += Ws_OnMessage;

            ws.Connect();
            ws.Send("{ \"head\":\"get\"}");
        }

        private void Ws_Send(string json)
        {
            ws.Send(json);
        }
        void Ws_OnMessage(object sender, MessageEventArgs e)
        {
            JObject jObject = JObject.Parse(e.Data);

            string head = (string)jObject["head"];
            if (head == "accept_message")
                addMessage((string)jObject["message"], (string)(jObject["_id"]["$oid"]));

            if (head == "accept_messages")
            {
                JArray arr = (JArray)jObject["messages"];
                var reverse = arr.Reverse();
                foreach (JObject value in reverse)
                    addMessage((string)value["message"], (string)(value["_id"]["$oid"]));
            }

            if (head == "delete")
                deleteMessage((string)(jObject["_id"]["$oid"]));
        }

            

        private void addMessage(string message, string id)
        {
            idlist.Add(id);
            flowLayoutPanel1.Invoke((MethodInvoker)(() =>
            {
                var container = CreateMessageContainer(message, id);
                flowLayoutPanel1.Controls.Add(container);
            }));
        }

        private void deleteMessage(string id)
        {
            int position = idlist.IndexOf(id);
            if (position < 0) return;

            flowLayoutPanel1.Invoke((MethodInvoker)(() =>
            {
                flowLayoutPanel1.Controls.RemoveAt(position);
            }));
            idlist.Remove(id);
        }
        private void sendMessage()
        {
            string message = textBox1.Text;
            if (message == "") return;

            Ws_Send("{\"head\": \"send\", \"message\":\"" + message + "\"}");
            textBox1.Text = "";
        }
        private void button1_Click(object sender, EventArgs e)
        {
            sendMessage();
        }

        private void textBox1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.SuppressKeyPress = true;
                sendMessage();
            }
        }
    }
}
