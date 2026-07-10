import React from 'react'
import ReactDOM from 'react-dom/client'
import App from './App'
import './styles.css'
import { connectPluginBridge } from './plugin/bridge'

// No-op in the browser; wires the WebView<->plugin bridge when hosted in JUCE.
connectPluginBridge()

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
)
